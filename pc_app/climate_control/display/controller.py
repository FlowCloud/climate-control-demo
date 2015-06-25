""" This module will act as MVC controller, it will sit between model and view

"""


import logging
import collections
import datetime
from PySide.QtCore import QObject, QTimer, Slot
from .view import DisplayView
from .view import DeviceEnum
from common.model import ClimateControlModel, FlowLibraryError, LoginFailed, ControllerNotFound, \
    UserList
from common.message_parse import MeasurementEvent, RelayStatusEvent, DeviceStatusEvent
import common.ui.strings as UIStrings
from common.miscellaneous import get_app_version


LOGGER = logging.getLogger(__name__)

# Constants
REFRESH_GRAPH_TIMEOUT = 1000
DEFAULT_TEMPERATURE_THRESHOLD = 25.0
DEFAULT_HUMIDITY_THRESHOLD = 30.0
# graph window size in seconds
GRAPH_WINDOW_SIZE = 60


class DisplayController(QObject):
    """ Controller class for the display app

    Acts as a coordinator between ClimateControlModel and DisplayView
    """
    def __init__(self, parent=None):
        super(DisplayController, self).__init__(parent)
        _, external_version = get_app_version()
        self.__display_app = DisplayView(external_version)
        self.__display_app.init_login_view(UserList().user_names)
        self.__display_app.register_login_callback(self.__login_btn_clicked)
        self.__display_app.close_signal.connect(self.__close)

        self.__model = None
        self.__refresh_graph_timer = QTimer(self)
        self.__refresh_graph_timer.timeout.connect(self.__speculate_data)
        self.temperature_data = collections.OrderedDict()
        self.humidity_data = collections.OrderedDict()
        self.temperature = None
        self.humidity = None
        self.__display_view_initialized = False

    @Slot(str, str)
    def __login_btn_clicked(self, user_name, password):
        """ Slot for the login button

         Tries to create model with the provided user_name and password

        :param str user_name: flow user name
        :param str password: flow user password
        """
        # clear any old status
        self.__display_app.login_view_show_error(None)

        LOGGER.debug("Trying to login with username = {}".format(user_name))

        try:
            # try to initialize model
            self.__model = ClimateControlModel()
            self.__model.connection_status.connect(self.__connection_status_result)
            try:
                self.__model.initialize(user_name, password)
            except LoginFailed:
                self.__display_app.login_view_show_error(UIStrings.LOGIN_FAILED)
            except ControllerNotFound:
                self.__display_app.login_view_show_error(UIStrings.CONTROLLER_DEVICE_NOT_FOUND)
            else:
                # Save username if no exception comes
                UserList().add_name(user_name)
                self.__model.setting_received.connect(self.__get_setting_response)
                # define slot for receiving events from Controller
                self.__model.measurement_changed.connect(self.__measurement_changed)
                self.__model.device_status_changed.connect(self.__device_status_changed)
                self.__model.relay_status_changed.connect(self.__relay_status_changed)
                self.__model.controller_status.connect(self.__controller_status)
                self.__model.latency_changed.connect(self.__latency_changed)
                self.__model.get_settings()
        except FlowLibraryError:
            self.__display_app.login_view_show_error(UIStrings.FLOW_LIBRARY_ERROR)

    @Slot(dict)
    def __get_setting_response(self, result):
        """ Slot for the get setting response from ClimateControlModel

        Initializes display view with received setting or shows error
        :param dict result: dictionary of the setting to be shown on configuration tab
        """
        if not self.__display_view_initialized:
            self.__display_app.close_login_view()
            self.__display_app.init_display_view(result["setting"])
            self.__display_view_initialized = True
            if result["error"]:
                self.__display_app.update_status_bar(UIStrings.SETTING_NOT_RECEIVED)
                LOGGER.error(result["error"])

        else:
            self.__display_app.update_settings(result["setting"])

    @Slot(dict)
    def __connection_status_result(self, connection_status):
        """Slot function for connection status result

        :param dict connection_status: dictionary containing network and internet status
        """
        status = UIStrings.OK
        color = "green"
        if not connection_status["network"]:
            color = "red"
            status = UIStrings.NETWORK_DOWN
        elif not connection_status["internet"]:
            status = UIStrings.INTERNET_DOWN
            color = "red"
        self.__display_app.set_connection_status(status, color)

    @Slot()
    def __close(self):
        """ Slot function when view gets closed

        Tries to de-initialize the model and stop refresh graph timer if active
        """
        if self.__refresh_graph_timer.isActive():
            self.__refresh_graph_timer.stop()

        LOGGER.debug("closing model")
        if self.__model:
            self.__model.close()

    @Slot(MeasurementEvent)
    def __measurement_changed(self, event):
        """ Slot function which receives measurement event from model

        :param MeasurementEvent event: Measurement Event object received from model
        """
        if self.__refresh_graph_timer.isActive():
            self.__refresh_graph_timer.stop()
        self.temperature = event.temperature
        self.humidity = event.humidity
        self.__update_data(self.temperature, self.humidity)
        self.__display_app.plot_graph(self.temperature_data, self.humidity_data)
        self.__refresh_graph_timer.start(REFRESH_GRAPH_TIMEOUT)

    @Slot()
    def __speculate_data(self):
        """ This function is a slot for refresh_graph timeout event

        It updates measurement data with last received measurement values
        """
        self.__update_data(self.temperature, self.humidity)
        self.__display_app.plot_graph(self.temperature_data, self.humidity_data)
        self.__refresh_graph_timer.start(REFRESH_GRAPH_TIMEOUT)

    def __update_data(self, temperature, humidity):
        """  Updates measurement data
        :param float temperature: temperature value
        :param float humidity: humidity value
        """
        current_time = datetime.datetime.now()
        if not (self.temperature_data and self.humidity_data):
            # fill the dictionary with last GRAPH_WINDOW_SIZE seconds of data
            for i in range(GRAPH_WINDOW_SIZE, 0, -1):
                self.temperature_data[current_time - datetime.timedelta(seconds=i)] = 0
                self.humidity_data[current_time - datetime.timedelta(seconds=i)] = 0

        self.temperature_data[current_time] = temperature
        self.humidity_data[current_time] = humidity
        if current_time - self.temperature_data.keys()[0] > datetime.\
                timedelta(seconds=GRAPH_WINDOW_SIZE):
            del self.temperature_data[self.temperature_data.keys()[0]]
            del self.humidity_data[self.humidity_data.keys()[0]]

    @Slot(str)
    def __controller_status(self, status):
        """ Slot function which receives controller device status from model

        It configures refresh graph timer according to controller device status
        :param str status: controller device status
        """
        LOGGER.debug("Controller status received - {}".format(status))
        if self.__display_app:
            self.__display_app.\
                update_device_status(DeviceEnum.controller, status == "ONLINE")
            # Latency is not applicable if controller is OFFLINE
            if status == "OFFLINE":
                self.__display_app.update_latency(None)
                LOGGER.debug("Latency set to NA")

            if status == "ONLINE" and not self.__refresh_graph_timer.isActive():
                self.__refresh_graph_timer.start(REFRESH_GRAPH_TIMEOUT)
                LOGGER.debug("Refresh graph timer started")

            elif status == "OFFLINE" and self.__refresh_graph_timer.isActive():
                self.__refresh_graph_timer.stop()
                LOGGER.debug("Refresh graph timer stopped")

            elif status == "ONLINE" and self.__refresh_graph_timer.isActive():
                LOGGER.debug("Refresh graph timer is already active")

            elif status == "OFFLINE" and not self.__refresh_graph_timer.isActive():
                LOGGER.debug("Refresh graph timer is already inactive")

        else:
            LOGGER.debug("Controller status received before display window is initialized")

    @Slot(DeviceStatusEvent)
    def __device_status_changed(self, event):
        """ Slot function which receives sensor and actuator status from model

        :param DeviceStatusEvent event: DeviceStatusEvent object
        """
        if self.__display_app:
            self.__display_app.update_device_status(DeviceEnum.sensor, event.sensor_alive)
            self.__display_app.update_device_status(DeviceEnum.actuator, event.actuator_alive)

        else:
            LOGGER.debug("Device(sensor and actuator) status received before display window is "
                         "initialized")

    @Slot(RelayStatusEvent)
    def __relay_status_changed(self, event):
        """ Slot function which receives relay status event from model

        :param RelayStatusEvent event: RelayStatusEvent object
        """
        if self.__display_app:
            self.__display_app.update_relay_status(event)

        else:
            LOGGER.debug("Relay status received before display window is initialized")

    @Slot(float)
    def __latency_changed(self, latency):
        """ Slot function which receives latency from model

        :param float latency: latency value
        """
        if self.__display_app:
            self.__display_app.update_latency(latency)
