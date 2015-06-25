""" This module will act as MVC model and represent the climate control system
It will talk with flow library

"""


import os
import logging
import datetime
from PySide.QtCore import QThreadPool, QObject, QRunnable, Signal, Slot, QTimer
from libflow.flow import flow_initialize, FlowUser, FlowException, FlowMessage
import xmltodict
from xml.parsers.expat import ExpatError
from xml.etree.ElementTree import Element, SubElement, tostring
from .message_parse import ControllerEventFactory, MeasurementEvent, HeartBeatEvent, \
    RelayStatusEvent, DeviceStatusEvent, ControllerCommand, ControllerResponse, \
    ControllerCommandEnum, ControllerResponseEnum
from .connection_status import NetworkMonitor

LOGGER = logging.getLogger(__name__)
THREAD_POOL_MAX_THREADS = 5
CONTROLLER_DEVICE_TYPE = "ClimateControlDemoController"
# Values above MAX_LATENCY_VALUE are ignored in latency calculation as they are likely to be wrong
MAX_LATENCY_VALUE = 10
# Message expiry in seconds
MESSAGE_EXPIRY_TIME = 20
# sends ping message every 20 seconds
LATENCY_PERIOD = 20


def get_server_details():
    """ Get server url, key and secret from config file

    :return str server_url: server url
    :return str server_key: server oauth key
    :return str server_secret: server oauth secret
    """
    flow_url = ""
    flow_key = ""
    flow_secret = ""
    filename = os.path.join(os.path.dirname(__file__), os.pardir, "libflow", "flow_config.cnf")
    with open(filename, "r") as config_file:
        for line in config_file:
            key, val = line.split("=")
            if key == "FLOW_SERVER_URL":
                flow_url = val.strip()
            elif key == "FLOW_SERVER_KEY":
                flow_key = val.strip()
            elif key == "FLOW_SERVER_SECRET":
                flow_secret = val.strip()

    return flow_url, flow_key, flow_secret


class FlowLibraryError(Exception):
    """ Exceptions raised when there is some library error
    """
    pass


class LoginFailed(Exception):
    """ Exception raised when login fails
    """
    pass


class ControllerNotFound(Exception):
    """ Exception raised when controller device not found
    """
    pass


class InvalidCommand(Exception):
    """ Exception raised when invalid command is asked to be sent
    """
    pass


class WorkerSignal(QObject):
    """ Class for Signal raised by  WorkerThread
    """
    signal = Signal(dict)


class MessageReceived(QObject):
    """ Class for signal raised when message is received
    """
    signal = Signal(str)


class WorkerThread(QRunnable):
    """ Worker thread for running the flow functions in thread pool

    """
    def __init__(self, worker_func, slot_func, **kargs):
        """ WorkerThread constructor

        :param worker_func: function to be executed in separate thread
        :param slot_func: response slot for the signal raised by worker_func
        :param kargs: arguments to worker_func
        """
        super(WorkerThread, self).__init__()
        self.__response = WorkerSignal()
        self.__func = worker_func
        self.__kargs = kargs
        # pylint: disable=star-args
        self.__response.signal.connect(lambda kargs: slot_func(**kargs))
        # pylint: enable=star-args

    def run(self):
        self.__func(self.__response.signal, **self.__kargs)


# All Worker functions
def get_setting_work(signal, device, key):
    """  Worker function for get setting of the specified key

    :param Signal signal: response signal to be raised if any error
    :param FlowDevice device: flow_device object for calling flow functions
    :param str key: key of the setting to be retrieved
    """
    result = {}
    value = None
    try:
        value = device.get_setting(key)
    except FlowException as flow_error:
        result["error"] = flow_error.message
        result["connection_error"] = flow_error.connection_error
        LOGGER.exception(flow_error)
    signal.emit({"setting": value, "error": result})


def save_setting_work(signal, device, key, value):
    """  Worker function for get setting of the specified key

    :param Signal signal: response signal:  dictionary of received value and error
    :param FlowDevice device: flow_device object for calling flow functions
    :param str key: key of the setting to be retrieved
    """
    result = {}
    try:
        device.save_setting(key, value)
    except FlowException as flow_error:
        result["error"] = {"error": flow_error.message,
                           "connection_error": flow_error.connection_error}
        LOGGER.exception(flow_error)
    signal.emit(result)


def send_command_work(signal, user, device, message):
    """ Worker function for sending message to device

    :param Signal signal: response signal to be raised if any error
    :param FlowUser user: user object
    :param FlowDevice device: device object
    :param str message: message to be sent
    """
    result = {}
    try:
        user.send_message_to_device(device, message, MESSAGE_EXPIRY_TIME)
    except FlowException as flow_error:
        result["error"] = {"error": flow_error.message,
                           "connection_error": flow_error.connection_error}
        LOGGER.exception(flow_error)
    signal.emit(result)


def check_connection_status_work(signal):
    """ Worker function for checking the connection status

    :param Signal signal: response signal
    """
    network_monitor = NetworkMonitor()
    connection_status = network_monitor.get_connection_status()
    signal.emit(connection_status)


class Latency(object):
    """ Latency class responsible for calculating exponential moving average latency value

    """
    def __init__(self):
        """ Constructor for Latency class
        """
        self.__prev_average = None
        self.exponential_moving_average = None

# pylint: disable=invalid-name
    def calculate_exponential_moving_average(self, value):
        """ Calculates exponential moving average

        :param float value: latest value to be considered in exponential moving average
        :return: exponential moving average
        :rtype: float
        """
        # alpha is weighting attached to current value. Calculated considering 10 values:
        # 2/(1 + N) = 2/(1 + 10) = 0.18
        alpha = 0.18
        if self.__prev_average:
            self.exponential_moving_average = value*alpha + self.__prev_average * (1 - alpha)
        else:
            self.exponential_moving_average = value
        self.__prev_average = self.exponential_moving_average
        return self.exponential_moving_average

# pylint: enable=invalid-name


# pylint: disable=too-many-public-methods
class ClimateControlModel(QObject):
    """ Model class representing the climate control system

    Talks with flow, fetches required data from flow, caches some of the data
    """
    __message_received_signal = None
    setting_received = Signal(dict)
    save_setting_result = Signal(dict)
    measurement_changed = Signal(MeasurementEvent)
    relay_status_changed = Signal(RelayStatusEvent)
    device_status_changed = Signal(DeviceStatusEvent)
    controller_status = Signal(str)
    command_response_received = Signal(dict)
    command_sending_result = Signal(dict)
    connection_status = Signal(dict)
    latency_changed = Signal(float)

    def __init__(self):
        """ ClimateControlModel constructor

        Initializes Flow library, thread pool
        """
        super(ClimateControlModel, self).__init__()
        self.__thread_pool = QThreadPool()
        self.__thread_pool.setMaxThreadCount(THREAD_POOL_MAX_THREADS)
        self.__flow_user = None
        self.__controller_device = None
        self.__checking_connection_status = False
        self.__last_measurement_event = None
        self.__last_relay_status_event = None
        self.__last_device_status_event = None
        self.__controller_heartbeat = None
        self.latency = Latency()
        self.__latency_timer = QTimer(self)
        self.__latency_timer.timeout.connect(self.__latency_timer_expired)
        ClimateControlModel.__message_received_signal = MessageReceived()
        ClimateControlModel.__message_received_signal.signal.connect(self.__message_received)

    def initialize(self, user_name, password):
        """ Initializes the system

        Initializes flow libraries and connects to server
        Logs in as user and tries to find the controller device, caches if found
        :param str user_name: flow user name
        :param str password: flow user password
        :raises ControllerNotFound, LoginFailed, FlowLibraryError
        """
        flow_url, flow_key, flow_secret = get_server_details()
        try:
            flow_initialize(flow_url, flow_key, flow_secret)
        except FlowException as flow_error:
            if flow_error.connection_error:
                self.check_connection_status()
            LOGGER.exception(flow_error)
            raise FlowLibraryError("Failed to initialize flow library")

        try:
            self.__flow_user = FlowUser(user_name, password)
        except FlowException as flow_error:
            if flow_error.connection_error:
                self.check_connection_status()
            LOGGER.exception(flow_error)
            raise LoginFailed()
        except ValueError as value_error:
            LOGGER.exception(value_error)
            raise LoginFailed()

        try:
            self.__controller_device = self.__flow_user.find_device(CONTROLLER_DEVICE_TYPE)
        except FlowException as flow_error:
            if flow_error.connection_error:
                self.check_connection_status()
            LOGGER.exception(flow_error)
            raise ControllerNotFound()

        # enable message reception
        self.__flow_user.enable_message_reception(ClimateControlModel.message_received_callback)

    def close(self):
        """ De-initialize the model

        This function won't return till all the worker thread finish their job
        """
        LOGGER.debug("waiting for threads in thread pool")
        self.__thread_pool.waitForDone()
        LOGGER.debug("thread pool exit done")
        if self.__flow_user:
            self.__flow_user.logout()

    def get_settings(self):
        """ Get setting for the controller device

        Schedules a worker thread for getting setting
        """
        runnable = WorkerThread(get_setting_work, self.__settings_received,
                                device=self.__controller_device,
                                key=ControllerSetting.controller_config_key)
        self.__thread_pool.start(runnable)

    def save_settings(self, setting):
        """ Save setting of controller device

        :param ControllerSetting setting: setting object to be saved in KVS
        """
        setting_xml = setting.to_xml()
        LOGGER.debug("Trying to save setting xml = {}".format(setting_xml))
        runnable = WorkerThread(save_setting_work, self.__save_settings_result,
                                device=self.__controller_device,
                                key=ControllerSetting.controller_config_key,
                                value=setting_xml)
        self.__thread_pool.start(runnable)

    def send_command(self, command):
        """ Send command to the controller device

        :param ControllerCommandEnum command: command to be sent
        """
        if command not in ControllerCommandEnum:
            raise InvalidCommand("Command not supported")

        LOGGER.info("sending command {}".format(command.value))
        command_xml = ControllerCommand(command).xml
        LOGGER.debug("command xml {}".format(command_xml))
        runnable = WorkerThread(send_command_work, self.__command_sent_result,
                                user=self.__flow_user, device=self.__controller_device,
                                message=command_xml)
        self.__thread_pool.start(runnable)

    def check_connection_status(self):
        """ Starts worker thread for checking connection status
        """
        if not self.__checking_connection_status:
            self.__checking_connection_status = True
            runnable = WorkerThread(check_connection_status_work, self.__connection_status_result)
            self.__thread_pool.start(runnable)

    @Slot(str, str)
    def __settings_received(self, setting, error):
        """ Slot function called when get setting worker thread finishes the task

        :param str setting: received setting from flow
        :param str error: error string if any
        """
        parsed_setting = None
        if not error:
            LOGGER.debug("Received setting xml = {}".format(setting))
            try:
                parsed_setting = ControllerSetting(setting_xml=setting)
                if not self.__controller_heartbeat:
                    # start heartbeat timer with twice the heartbeat period so that even if some
                    # heartbeat message failed to receive, it should wait for another
                    # heartbeat message
                    self.__controller_heartbeat = \
                        HeartBeatTimer(parsed_setting.controller_heartbeat*2*1000,
                                       self.__heartbeat_timer_status_changed)
                else:
                    # if heartbeat already exist then restart with new time
                    self.__controller_heartbeat.change_timeout_period(
                        parsed_setting.controller_heartbeat*2*1000)
            except ValueError as value_error:
                LOGGER.exception(value_error)
                error = "Invalid setting XML received"
        else:
            if error["connection_error"]:
                self.check_connection_status()

        self.setting_received.emit({"setting": parsed_setting, "error": error})

    @Slot(str)
    def __save_settings_result(self, error=None):
        """ Slot function for the save setting worker function result

        :param str error: error string if any
        """
        self.save_setting_result.emit({"error": error})
        if error:
            LOGGER.debug("Save setting failed: {}".format(error))
            if error["connection_error"]:
                self.check_connection_status()
        else:
            # if KVS updated then send message to device to update the settings
            self.send_command(ControllerCommandEnum.retrieve_settings)
            LOGGER.debug("Setting saved to KVS")

    @Slot(bool, bool)
    def __connection_status_result(self, network, internet):
        """ Slot function for connection status result
        :param bool network: False if network is down
        :param bool internet: False if internet is down
        """
        self.__checking_connection_status = False
        connection_status = {"network": network, "internet": internet}
        self.connection_status.emit(connection_status)

    @Slot(str)
    def __command_sent_result(self, error=None):
        """ Slot function called at the end of sending command

        :param str error: error string if any
        """
        if error:
            self.command_sending_result.emit({"error": error})
            LOGGER.debug("Message sending failed: {}".format(error))
            if error["connection_error"]:
                self.check_connection_status()
        else:
            LOGGER.debug("Message sending success")

# pylint: disable=invalid-name
    @Slot(str)
    def __heartbeat_timer_status_changed(self, status):
        """ Slot function called when heartbeat timer changes its status(start, expire etc)

        It receives controller status. on timer expire-controller status = "OFFLINE"
        on timer refresh(start)- controller status = "ONLINE"
        :param str status: "ONLINE" or "OFFLINE" status
        """
        self.controller_status.emit(status)
        if status == "OFFLINE":
            # timer expired so there might be something wrong with the
            # network or internet so check connection status
            self.check_connection_status()
            LOGGER.debug("Latency timer stopped")
            self.__latency_timer.stop()
            if self.__last_device_status_event:
                # if controller is OFFLINE, make sensor and actuator also OFFLINE
                self.__last_device_status_event.sensor_alive = False
                self.__last_device_status_event.actuator_alive = False
                self.device_status_changed.emit(self.__last_device_status_event)
        else:
            self.__latency_timer.start(LATENCY_PERIOD*1000)
            LOGGER.debug("Latency timer started")
# pylint: enable=invalid-name

    @Slot()
    def __latency_timer_expired(self):
        """ Slot called on the expiry of latency timer

        """
        # send ping message to controller and calculate latency based on when it is received
        self.send_command(ControllerCommandEnum.ping)

    def __handle_command_response(self, response_dict):
        """ Parse the received response and emit signal if valid response found

        :param dict response_dict: response xml dictionary
        """
        try:
            response = ControllerResponse(response_dict)
            LOGGER.info("received response {}".format(response.response.value))
            if response.response == ControllerResponseEnum.ping:
                # round_trip_time is the difference between current local time and timestamp
                # when message was sent, this includes any processing delay on controller side
                round_trip_time = (datetime.datetime.utcnow() - response.sent_time).total_seconds()
                LOGGER.debug("round_trip_time: {}".format(round_trip_time))
                # Ignore value where round_trip_time > MAX_LATENCY_TIME
                if round_trip_time <= MAX_LATENCY_VALUE:
                    latency = self.latency.calculate_exponential_moving_average(round_trip_time)
                else:
                    latency = round_trip_time
                self.latency_changed.emit(latency)
                LOGGER.debug("Latency: {}".format(latency))
            else:
                self.command_response_received.emit({"response": response.response})

            # if retrieve_settings_success is received, get the settings again as it might
            # be update by some other app
            # for e.g. admin app updating threshold and display app updating its values accordingly
            if response.response == ControllerResponseEnum.retrieve_settings_success:
                self.get_settings()
        except ValueError as error:
            LOGGER.exception(error)

    def __process_heartbeat_event(self, event):
        """ Processes HeartBeat event data

        :param HeartBeatEvent event: HeartBeat event object
        """
        if self.__last_measurement_event:
            if not event.measurement_data == self.__last_measurement_event:
                self.measurement_changed.emit(event.measurement_data)
                self.__last_measurement_event = event.measurement_data
            else:
                LOGGER.debug("Ignoring measurement data")
        else:
            self.measurement_changed.emit(event.measurement_data)
            self.__last_measurement_event = event.measurement_data

        LOGGER.debug("Heartbeat Temp: {} Humidity: {} ".format(event.measurement_data.temperature,
                                                               event.measurement_data.humidity))

        if self.__last_relay_status_event:
            if not event.relay_status == self.__last_relay_status_event:
                self.relay_status_changed.emit(event.relay_status)
                self.__last_relay_status_event = event.relay_status
            else:
                LOGGER.debug("Ignoring relay status data")
        else:
            self.relay_status_changed.emit(event.relay_status)
            self.__last_relay_status_event = event.relay_status

        if self.__last_device_status_event:
            if not event.device_status == self.__last_device_status_event:
                self.device_status_changed.emit(event.device_status)
                self.__last_device_status_event = event.device_status
            else:
                LOGGER.debug("Ignoring device status data")
        else:
            self.device_status_changed.emit(event.device_status)
            self.__last_device_status_event = event.device_status

        LOGGER.debug("Heartbeat Temp: {} Humidity: {} "
                     "Relay 1 ON: {} Relay 2 ON: {} "
                     "Sensor: {} Actuator: {}".format(event.measurement_data.temperature,
                                                      event.measurement_data.humidity,
                                                      event.relay_status.relay_1_on,
                                                      event.relay_status.relay_2_on,
                                                      event.device_status.sensor_alive,
                                                      event.device_status.actuator_alive))

    def __handle_event(self, message_dict):
        """ Create event object according to event type and handle the event

        :param dict message_dict: message content in the dictionary format
        """
        try:
            event = ControllerEventFactory.create_event(message_dict)
            if self.__controller_heartbeat:
                self.__controller_heartbeat.refresh_timer()

            if isinstance(event, MeasurementEvent):
                self.measurement_changed.emit(event)
                self.__last_measurement_event = event
                LOGGER.debug("Measurement Temp: {} Humidity: {}".format(event.temperature,
                                                                        event.humidity))
            elif isinstance(event, RelayStatusEvent):
                self.relay_status_changed.emit(event)
                self.__last_relay_status_event = event
                LOGGER.debug("RelayStatus Relay 1 ON: {} Relay 2 ON: {}".format(event.relay_1_on,
                                                                                event.relay_2_on))
            elif isinstance(event, DeviceStatusEvent):
                self.device_status_changed.emit(event)
                self.__last_device_status_event = event
                LOGGER.debug("DeviceStatus Sensor: {} Actuator: {}".format(event.sensor_alive,
                                                                           event.actuator_alive))
            elif isinstance(event, HeartBeatEvent):
                self.__process_heartbeat_event(event)

        except ValueError as value_error:
            LOGGER.exception(value_error)

    @Slot(str)
    def __message_received(self, message_content):
        """ Slot function called when message is received by callback function

        This function parses message_content(xml_string) into dictionary
        :param str message_content: message content which is xml string
        """
        LOGGER.debug("Received message {}".format(message_content))
        # We have received a message from controller means network and internet are up
        status = {"network": True, "internet": True}
        self.connection_status.emit(status)
        try:
            message_dict = xmltodict.parse(message_content)
            root_tag = message_dict.keys()[0]
            if root_tag == "response":
                self.__handle_command_response(message_dict)
            elif root_tag == "event":
                self.__handle_event(message_dict)
            else:
                LOGGER.error("Unsupported message received {}".format(message_content))
        except (ExpatError, KeyError) as error:
            LOGGER.exception("Error in parsing xml {} xml={}".
                             format(error.message, message_content))

    @staticmethod
    def message_received_callback(flow_message):
        """ Callback called by library, libflow expects this to be a static class method

        :param flow_message: message object from library
        """
        message = FlowMessage(flow_message)
        message_content = message.get_message_content()
        ClimateControlModel.__message_received_signal.signal.emit(message_content)

# pylint: enable=too-many-public-methods


# pylint: disable=too-many-instance-attributes
class ControllerSetting(object):
    """ Controller setting class

    """
    controller_config_key = "ControllerConfig"
    threshold_orientation_above = "ABOVE"
    threshold_orientation_below = "BELOW"

    def __init__(self, setting=None, setting_xml=None):
        """ Creates setting object, pass only one parameter, setting will be preferred

        :param dict setting: dictionary of the setting
        :param str setting_xml: setting xml string
        :raises ValueError: when error in parsing xml
        """
        if setting:
            self.controller_heartbeat = setting["controller_heartbeat"]
            self.sensor_heartbeat = setting["sensor_heartbeat"]
            self.actuator_heartbeat = setting["actuator_heartbeat"]
            self.temperature_threshold = setting["temperature_threshold"]
            self.humidity_threshold = setting["humidity_threshold"]
            self.temperature_orientation = setting["temperature_orientation"]
            self.humidity_orientation = setting["humidity_orientation"]
            self.temperature_read_interval = setting["temperature_read_interval"]
            self.humidity_read_interval = setting["humidity_read_interval"]
            self.temperature_read_delta = setting["temperature_read_delta"]
            self.humidity_read_delta = setting["humidity_read_delta"]
        elif setting_xml:
            # sample xml format
            # <ControllerConfig>
            #     <version>1.0</version>
            #     <TemperatureThreshold>25.00</TemperatureThreshold>
            #     <HumidityThreshold>35.00</HumidityThreshold>
            #     <TemperatureOrientation>ABOVE</TemperatureOrientation>
            #     <HumidityOrientation>BELOW</HumidityOrientation>
            #     <HeartBeat>15000</HeartBeat>
            #     <SensorConfig>
            #         <HeartBeat>15000</HeartBeat>
            #         <TemperatureReadInterval>1000</TemperatureReadInterval>
            #         <HumidityReadInterval>2500</HumidityReadInterval>
            #         <TemperatureReadDelta>0.5</TemperatureReadDelta>
            #         <HumidityReadDelta>2</HumidityReadDelta>
            #     </SensorConfig>
            #     <ActuatorConfig>
            #         <HeartBeat>15000</HeartBeat>
            #     </ActuatorConfig>
            # </ControllerConfig>
            try:
                setting_dict = xmltodict.parse(setting_xml)
            except ExpatError as error:
                raise ValueError("Setting xml parsing error {}".format(error.message))

            try:
                self.temperature_threshold = \
                    float(setting_dict["ControllerConfig"]["TemperatureThreshold"])
                self.humidity_threshold = \
                    float(setting_dict["ControllerConfig"]["HumidityThreshold"])
                if setting_dict["ControllerConfig"]["TemperatureOrientation"] not in \
                    [ControllerSetting.threshold_orientation_above,
                     ControllerSetting.threshold_orientation_below]:
                    raise ValueError("Setting xml, invalid TemperatureOrientation")
                else:
                    self.temperature_orientation = \
                        setting_dict["ControllerConfig"]["TemperatureOrientation"]
                if setting_dict["ControllerConfig"]["HumidityOrientation"] not in \
                    [ControllerSetting.threshold_orientation_above,
                     ControllerSetting.threshold_orientation_below]:
                    raise ValueError("Setting xml, invalid HumidityOrientation")
                else:
                    self.humidity_orientation = \
                        setting_dict["ControllerConfig"]["HumidityOrientation"]
                # Heartbeat saved in xml is in msec, converting to sec
                self.controller_heartbeat = \
                    int(setting_dict["ControllerConfig"]["HeartBeat"])/1000
                self.temperature_read_interval = \
                    int(setting_dict["ControllerConfig"]["SensorConfig"]["TemperatureReadInterval"])
                self.humidity_read_interval = \
                    int(setting_dict["ControllerConfig"]["SensorConfig"]["HumidityReadInterval"])
                self.temperature_read_delta = \
                    float(setting_dict["ControllerConfig"]["SensorConfig"]["TemperatureReadDelta"])
                self.humidity_read_delta = \
                    float(setting_dict["ControllerConfig"]["SensorConfig"]["HumidityReadDelta"])
                self.sensor_heartbeat = \
                    int(setting_dict["ControllerConfig"]["SensorConfig"]["HeartBeat"])/1000
                self.actuator_heartbeat = \
                    int(setting_dict["ControllerConfig"]["ActuatorConfig"]["HeartBeat"])/1000
            except KeyError:
                raise ValueError("Setting xml parsing error, tag not found")

    def to_xml(self):
        """ Create setting xml

        :return: setting xml string
        :rtype: str
        """
        root = Element("ControllerConfig")
        SubElement(root, "version").text = "1.0"
        SubElement(root, "TemperatureThreshold").text = "{:.2f}".format(self.temperature_threshold)
        SubElement(root, "HumidityThreshold").text = "{:.2f}".format(self.humidity_threshold)
        SubElement(root, "TemperatureOrientation").text = self.temperature_orientation
        SubElement(root, "HumidityOrientation").text = self.humidity_orientation
        SubElement(root, "HeartBeat").text = "{}".format(self.controller_heartbeat*1000)

        sensor_config = SubElement(root, "SensorConfig")
        SubElement(sensor_config, "HeartBeat").text = "{}".format(self.sensor_heartbeat*1000)
        SubElement(sensor_config, "TemperatureReadInterval").text = \
            "{}".format(self.temperature_read_interval)
        SubElement(sensor_config, "HumidityReadInterval").text = \
            "{}".format(self.humidity_read_interval)
        SubElement(sensor_config, "TemperatureReadDelta").text = \
            "{}".format(self.temperature_read_delta)
        SubElement(sensor_config, "HumidityReadDelta").text = \
            "{}".format(self.humidity_read_delta)

        actuator_config = SubElement(root, "ActuatorConfig")
        SubElement(actuator_config, "HeartBeat").text = "{}".format(self.actuator_heartbeat*1000)
        return tostring(root)
# pylint: enable=too-many-instance-attributes


class UserList(object):
    """ UserList class, currently allows save/retrieve of user names
    from file username.txt

    """
    def __init__(self):
        self.__filename = os.path.join(os.path.dirname(__file__), os.pardir, "username.txt")
        self.user_names = []
        try:
            with open(self.__filename, "r") as username_file:
                user_names = username_file.readlines()
                user_names = [line.strip("\n") for line in user_names]
                self.user_names = user_names
        except IOError as error:
            LOGGER.exception(error)

    def add_name(self, user_name):
        """ Add the user name in a file if not present already

        :param str user_name: user name
        """
        if user_name not in self.user_names:
            try:
                with open(self.__filename, "a+") as username_file:
                    username_file.write("{}\n".format(user_name))
                    self.user_names.append(user_name)
            except IOError as error:
                LOGGER.exception(error)


class HeartBeatTimer(QObject):
    """ Class for HeartBeat event timer
    """
    __status_changed = Signal(str)

    def __init__(self, period, callback):
        """ Constructor for HeartBeatTimer class

        :param int period: timeout period in msec
        :param callback: callback function to be called when timer changes its status
        callback signature: callback(status)
        """
        super(HeartBeatTimer, self).__init__()
        self.__period = period
        self.is_alive = False
        self.__timer = QTimer(self)
        self.__timer.timeout.connect(self.__heartbeat_timer_expired)
        self.__timer.start(self.__period)
        self.__status_changed.connect(callback)
        self.__status_changed.emit("OFFLINE")

    def refresh_timer(self):
        """ Restart the heartbeat timer

        """
        self.__timer.start(self.__period)
        if not self.is_alive:
            self.is_alive = True
            self.__status_changed.emit("ONLINE")

    def change_timeout_period(self, period):
        """ Change the existing timeout period if not same

        :param int period: new timeout period in msec
        """
        if period != self.__period:
            self.__period = period
            self.__timer.start(self.__period)

    @Slot()
    def __heartbeat_timer_expired(self):
        """ Slot function for heartbeat timeout event

        """
        self.__timer.stop()
        if self.is_alive:
            self.is_alive = False
            self.__status_changed.emit("OFFLINE")
