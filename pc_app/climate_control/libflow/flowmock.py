""" Mocking for the flow.py wrapper, can be used if talking with libflow library has to be avoided

"""

import logging
import xmltodict
from xml.parsers.expat import ExpatError


LOGGER = logging.getLogger(__name__)


def flow_initialize(dummy_url, dummy_key, dummy_secret):
    """ Dummy initialization of flow library

    :param str url: ignored
    :param str key: ignored
    :param str secret: ignored
    """
    pass


class FlowException(Exception):
    """ FlowException handles the exceptions raised for Flow API's
    """
    def __init__(self, message, dummy_flowcore_error_code=None,
                 dummy_flowmessaging_error_code=None):
        super(FlowException, self).__init__(message)
        # set dummy error code and message
        self.error_code = -1
        self.error_code_message = "Dummy flow error"


class FlowMessage(object):
    """ FlowMessage contains the message sent by user or device.

    """
    def __init__(self, message):
        self.__message = message

    def get_message_content(self):
        """ Get message content

        :return: message content.
        """
        return self.__message


# TBD: need to make this singleton
class FlowUser(object):
    """ Class representing Flow User

    """
    def __init__(self, name, password):
        """ Mock function for constructing FlowUser

        :param str name: should be "test" or raises FlowException
        :param str password: should be "test" or raises FlowException
        :raises: FlowException if name and password is not "test"
        """
        if not (name == "test" and password == "test"):
            raise FlowException("Username or password incorrect")
        self.__message_received_callback = None

    def find_device(self, devicetype):
        """ Currently returns FlowDevice if devicetype is ClimateControlDemoController

        :raises: FlowException if valid device is not found
        :return: flow device object
        :rtype: FlowDevice
        """
        if devicetype == "ClimateControlDemoController":
            return FlowDevice("ClimateControlDemoController")
        else:
            raise FlowException("Device not found")

    def logout(self):
        """ Dummy logout

        """
        pass

    def enable_message_reception(self, callback_function):
        """ Set callback function to be called on reception of response message
        callback function is responsible for getting message content and parsing it

        :param callback_function: function to be called on reception of response  message
        """
        self.__message_received_callback = callback_function

    def __create_response_string(self, response):
        """ Create response xml

        :param str response: response string
        :return: response xml string
        :rtype: string
        """
        # time set is dummy for testing
        response_xml = "<response>"\
                       "<time type=\"datetime\">2014-11-28T15:30:09Z</time>"\
                       "<info>{}</info>"\
                       "</response>".format(response)
        return response_xml

    def __parse_command(self, command_xml):
        """ Parses the incoming xml and returns command_xml

        :param str command_xml: command xml
        :return: command string
        :rtype: string
        """
        message_dict = xmltodict.parse(command_xml)
        return message_dict["command"]["info"]

    def send_message_to_device(self, device, message):
        """ Send message to FlowDevice

        :param FlowDevice device: FlowDevice object
        :param str message: message string to be sent
        """
        if device:
            LOGGER.debug("-> {}".format(message))
            response = None
            # parse the command and sent appropriate response
            try:
                command = self.__parse_command(message)
                # except for update settings response is same as command
                if command == "RETRIEVE_SETTINGS":
                    response = "RETRIEVE_SETTINGS_SUCCESS"
                else:
                    response = command
            except (ExpatError, KeyError) as error:
                LOGGER.exception(error)

            if response and self.__message_received_callback:
                response_xml = self.__create_response_string(response)
                LOGGER.debug("<- {}".format(response_xml))
                self.__message_received_callback(response_xml)
        else:
            raise FlowException("FlowDevice object is None")


class FlowDevice(object):
    """ Class representing the flow device. currently acts as controller device

    """
    def __init__(self, flow_interface_device):
        """ constructs FlowDevice with flow_interface_device acting as type

        :param flow_interface_device: currently uses this for type
        """
        self.__flow_interface_device = flow_interface_device
        self.__setting = {"ControllerConfig":
                          "<ControllerConfig>"
                          "<version>1.0</version>"
                          "<TemperatureThreshold>25.00</TemperatureThreshold>"
                          "<HumidityThreshold>35.00</HumidityThreshold>"
                          "<TemperatureOrientation>ABOVE</TemperatureOrientation>"
                          "<HumidityOrientation>BELOW</HumidityOrientation>"
                          "<HeartBeat>15000</HeartBeat>"
                          "<SensorConfig>"
                          "<HeartBeat>15000</HeartBeat>"
                          "<TemperatureReadInterval>1000</TemperatureReadInterval>"
                          "<HumidityReadInterval>2500</HumidityReadInterval>"
                          "<TemperatureReadDelta>0.5</TemperatureReadDelta>"
                          "<HumidityReadDelta>2.0</HumidityReadDelta>"
                          "</SensorConfig>"
                          "<ActuatorConfig>"
                          "<HeartBeat>15000</HeartBeat>"
                          "</ActuatorConfig>"
                          "</ControllerConfig>"}

    def get_setting(self, key):
        """ If key is "ControllerConfig" returns config xml

        :param str key: current supported key string is "ControllerConfig"
        :raises: FlowException if un-supported key is passed
        :return: Configuration xml
        :rtype: string
        """
        if key in self.__setting:
            return self.__setting[key]
        else:
            raise FlowException("None settings retrieved")

    def save_setting(self, key, value):
        """Save setting

        :param str key: setting key
        :param str value: setting value
        """
        if key and value:
            self.__setting[key] = value
        else:
            raise FlowException("Invalid function parameters")
