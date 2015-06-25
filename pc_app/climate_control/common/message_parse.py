""" This module contains all controller device related parsing and data structures

"""

from enum import Enum
from datetime import datetime
from xml.etree.ElementTree import Element, SubElement, tostring


# constants
TIME_FMT = '%Y-%m-%dT%H:%M:%SZ'
TIME_FMT_MICROSECONDS = '%Y-%m-%dT%H:%M:%S.%fZ'


class ControllerEventFactory(object):
    """ Factory class for creating controller events
    """

    @staticmethod
    def create_event(event_dict):
        """ Creates event object according to event type from event_dict

        :param dict event_dict: event_dict which has parsed data from received message xml
        :return: controller event object
        :rtype: HeartBeatEvent, MeasurementEvent, RelayStatusEvent, DeviceStatusEvent
        :raises: ValueError: If event_dict failed to have entry for "type" tag
        """
        try:
            if event_dict["event"]["type"] == "HeartBeat":
                return HeartBeatEvent(event_dict)

            elif event_dict["event"]["type"] == "Measurement":
                return MeasurementEvent(event_dict)

            elif event_dict["event"]["type"] == "DeviceStatus":
                return DeviceStatusEvent(event_dict)

            elif event_dict["event"]["type"] == "RelayStatus":
                return RelayStatusEvent(event_dict)

            else:
                raise ValueError("Unsupported event type received")

        except KeyError:
            raise ValueError("Event xml doesn't have 'type' tag")


class MeasurementEvent(object):
    """ Class for measurement event
    """
    def __init__(self, event_dict):
        """ Measurement event constructor

        Initializes event parameters such as temperature and humidity
        :param dict event_dict: event dictionary which has parsed data from received message xml
        """
        try:
            self.time = datetime.strptime(event_dict["event"]["time"]["#text"],
                                          TIME_FMT)
            self.temperature = float(event_dict["event"]["info"]["Temperature"])
            self.humidity = float(event_dict["event"]["info"]["Humidity"])
        except KeyError:
            raise ValueError("Failed to parse measurement event")

    def __eq__(self, other):
        return self.temperature == other.temperature and self.humidity == other.humidity


class DeviceStatusEvent(object):
    """ Class for device status event
    """
    def __init__(self, event_dict):
        """ DeviceStatus event constructor

        :param dict event_dict: event dictionary which has parsed data from received message xml
        """
        try:
            self.time = datetime.strptime(event_dict["event"]["time"]["#text"],
                                          TIME_FMT)
            self.sensor_alive = DeviceStatusEvent.\
                __is_alive(event_dict["event"]["info"]["Sensor"])
            self.actuator_alive = DeviceStatusEvent.\
                __is_alive(event_dict["event"]["info"]["Actuator"])
        except KeyError:
            raise ValueError("Failed to parse device status event")

    def __eq__(self, other):
        return self.sensor_alive == other.sensor_alive and \
            self.actuator_alive == other.actuator_alive

    @staticmethod
    def __is_alive(device_status):
        """ Check whether device is ONLINE(ALIVE)

        :param str device_status: "DEAD" or "ALIVE"
        :return: True if device_status is "ALIVE"
        :rtype: bool
        :raises ValueError
        """
        if device_status not in ["DEAD", "ALIVE"]:
            raise ValueError("Invalid value for device status")
        return device_status == "ALIVE"


class RelayStatusEvent(object):
    """ Class representing relay status event
    """
    def __init__(self, event_dict):
        """ RelayStatus event constructor

        :param dict event_dict: event dictionary which has parsed data from received message xml
        """
        try:
            self.time = datetime.strptime(event_dict["event"]["time"]["#text"],
                                          TIME_FMT)
            self.relay_1_on = RelayStatusEvent.__is_on(
                event_dict["event"]["info"]["Relay_1"]["status"])
            self.relay_2_on = RelayStatusEvent.__is_on(
                event_dict["event"]["info"]["Relay_2"]["status"])
            self.relay_1_mode = event_dict["event"]["info"]["Relay_1"]["mode"]
            self.relay_2_mode = event_dict["event"]["info"]["Relay_2"]["mode"]
            self.__valid_mode()
        except KeyError:
            raise ValueError("Failed to parse relay status event")

    def __eq__(self, other):
        return self.relay_1_on == other.relay_1_on and \
            self.relay_1_mode == other.relay_1_mode and \
            self.relay_2_on == other.relay_2_on and \
            self.relay_2_mode == other.relay_2_mode

    def __valid_mode(self):
        """ Check whether relay mode is valid

        :raises ValueError
        """
        if self.relay_1_mode not in ["AUTO", "MANUAL"] or self.relay_2_mode not in \
                ["AUTO", "MANUAL"]:
            raise ValueError("Invalid value for relay mode")

    @staticmethod
    def __is_on(relay_status):
        """ Check whether relay is ON

        :param str relay_status: "ON" or "OFF"
        :return: True if relay_status is "ON"
        :rtype: bool
        :raises ValueError
        """
        if relay_status not in ["OFF", "ON"]:
            raise ValueError("Invalid value for relay status")
        return relay_status == "ON"


class HeartBeatEvent(object):
    """ Class representing Heartbeat event
    """
    def __init__(self, event_dict):
        """ Heartbeat event constructor

        :param event_dict: event dictionary which has parsed data from received message xml
        """
        try:
            self.time = datetime.strptime(event_dict["event"]["time"]["#text"],
                                          TIME_FMT)
            self.measurement_data = MeasurementEvent(event_dict)
            self.device_status = DeviceStatusEvent(event_dict)
            self.relay_status = RelayStatusEvent(event_dict)
        except KeyError:
            raise ValueError("Failed to parse heartbeat event")


class ControllerCommandEnum(Enum):
    """ Enum class for command string supported by controller

    """
    relay_1_on = "RELAY_1_ON"
    relay_1_off = "RELAY_1_OFF"
    relay_1_auto = "RELAY_1_AUTO"
    relay_2_on = "RELAY_2_ON"
    relay_2_off = "RELAY_2_OFF"
    relay_2_auto = "RELAY_2_AUTO"
    retrieve_settings = "RETRIEVE_SETTINGS"
    ping = "PING"


class ControllerResponseEnum(Enum):
    """ Enum class for command response string supported by controller

    """
    retrieve_settings_success = "RETRIEVE_SETTINGS_SUCCESS"
    retrieve_settings_failure = "RETRIEVE_SETTINGS_FAILURE"
    relay_1_on = "RELAY_1_ON"
    relay_1_off = "RELAY_1_OFF"
    relay_1_auto = "RELAY_1_AUTO"
    relay_2_on = "RELAY_2_ON"
    relay_2_off = "RELAY_2_OFF"
    relay_2_auto = "RELAY_2_AUTO"
    ping = "PING"


class ControllerCommand(object):
    """ Class for controller device command xml

    """

    def __init__(self, command):
        """ Create command xml from the command

        :param ControllerCommandEnum command: command enum
        """
        # command xml
        # <command>
        #     <time type="datetime">2014-11-28T15:30:09Z</time>
        #     <info>RETRIEVE_SETTINGS/RELAY_1_ON/RELAY_1_OFF/RELAY_2_ON/RELAY_2_OFF</info>
        # </command>
        #

        root = Element("command")
        time_tag = SubElement(root, "time", attrib={"type": "datetime"})
        time_tag.text = datetime.utcnow().strftime(TIME_FMT)
        SubElement(root, "info").text = "{}".format(command.value)
        if command == ControllerCommandEnum.ping:
            SubElement(root, "app_time").text = \
                datetime.utcnow().strftime(TIME_FMT_MICROSECONDS)
        self.type = command
        self.xml = tostring(root)


class ControllerResponse(object):
    """ Class for controller device response xml

    """

    def __init__(self, response_dict):
        """ Parse the xml dictionary and checks if it is valid response

        :param dict response_dict: dictionary of the response xml
        :raises ValueError: if any error in parsing response
        """
        # command xml
        # <response>
        #     <time type="datetime">2014-11-28T15:30:09Z</time>
        #     <info>RETRIEVE_SETTINGS_SUCCESS/RETRIEVE_SETTINGS_FAILURE/RELAY_1_ON/
        #           RELAY_1_OFF/RELAY_2_ON/RELAY_2_OFF</info>
        # </response>
        #
        try:
            # search if response is a valid supported string
            for enum in ControllerResponseEnum:
                if enum.value == response_dict["response"]["info"]:
                    self.response = enum
                    break
            else:
                raise ValueError("Unsupported response message received {}".
                                 format(response_dict["response"]["info"]))
            try:
                self.time = datetime.strptime(response_dict["response"]["time"]["#text"], TIME_FMT)
                # for ping command, controller copies the app_time tag and replies back
                # the same xml tag, this time is used for calculating round trip time
                if self.response == ControllerResponseEnum.ping:
                    self.sent_time = datetime.strptime(
                        response_dict["response"]["app_time"], TIME_FMT_MICROSECONDS)
                else:
                    self.sent_time = None
            except ValueError:
                raise ValueError("Response xml time parsing error")
        except KeyError:
            raise ValueError("Response xml parsing error, tag not found")
