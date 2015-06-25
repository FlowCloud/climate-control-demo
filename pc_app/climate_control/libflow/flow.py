""" This module contains implementation of classes which talk to libflow_interface library named
    FlowUser, FlowDevice, FlowMessage and also implementation of FlowCoreException and
    FlowMessagingException class

"""


import logging
import copy
import libflow_interface


LOGGER = logging.getLogger(__name__)


def flow_set_nvs_file(file_path):
    """ Sets flow nvs file path, default file is "libflow-nvs" in launch directory

    :param str file_path: nvs file path
    """
    libflow_interface.set_nvs_file(file_path)


def flow_initialize(url, key, secret):
    """ Initialize flow and connect to server

    :param str url: server url
    :param str key: OAUTH key
    :param str secret: OAUTH password
    """
    libflow_interface.initialize_flow()
    libflow_interface.connect_to_flow_server(url, key, secret)


class FlowException(Exception):
    """ FlowException is a base class for exceptions raised due to libflow API failure

    """
    def __init__(self, message, error_code=None, connection_error=False):
        """ Constructor for FlowException class

        :param str message: error string
        :param int error_code: error code set due to libflow(FlowCore and FlowMessaging) API failure
        """
        super(FlowException, self).__init__(message)
        self.error_code = error_code
        self.connection_error = connection_error


class FlowCoreException(FlowException):
    """ FlowCoreException handles the exceptions raised for FlowCore API

    """
    __flow_core_error_messages = [
        "No error",
        "Internal: an internal error occurred, this is a bug that should be reported to libflow "
        "developers",                                                                           # 1
        "Memory: there is not enough memory to fulfill this operation",                         # 2
        "Method unavailable: the requested method is not supported on this object",             # 3
        "Invalid argument: one of the method argument was invalid",                             # 4
        "Resource not found: the resource that was requested in not available",                 # 5
        "Network: failure to connect to the server",                                            # 6
        "Unauthorised: the FlowClient doesn't have the credentials to access this "
        "resource",                                                                             # 7
        "Conflict: the submitted data is in conflict with existing data on the webservice, "
        "look up the HTML documentation for more details",                                      # 8
        "Removed: the resource you're trying to access existed but has been removed",           # 9
        "Server: the server suffered an internal error",                                        # 10
        "Server busy: webservice cannot fulfill request at the moment",                         # 11
        "Time out: timed out while waiting for this request",                                   # 12
        "Anonymous: the FlowClient is not logged in, there is no current user or device",       # 13
        "Local storage: failure to store data in local file system",                            # 14
        "Version conflict: the server and client are not compatible, check that the client is",
        "up to date",                                                                           # 15
        "Concurrent modification: the object you're trying to use has been modified "
        "concurrently, get a new copy",                                                         # 16

        "Bad request invalid fields",                                                           # 17
        "Bad request invalid pin",                                                              # 18
        "Bad request insufficient funds",                                                       # 19
        "Bad request product not found",                                                        # 20
        "Bad request product not available",                                                    # 21
        "Bad request card not supported",                                                       # 22
        "Bad request invalid card number",                                                      # 23
        "Bad request invalid card expiry date",                                                 # 24
        "Bad request subscription invalid for user",                                            # 25
        "Bad request subscription already trialled",                                            # 26
        "Bad request subscription cannot be trialled",                                          # 27
        "Bad request subscription cannot be renewed",                                           # 28
        "Bad request duplicate email",                                                          # 29
        "Bad request account not found",                                                        # 30
        "Bad request voucher not valid",                                                        # 31
        "Bad request voucher already being processed",                                          # 32
        "Bad request content folder limit reached",                                             # 33
        "Bad request unknown",                                                                  # 34

        "Server time sync: the client is not time-synchronized with the server, "
        "FlowClient_SynchronizeServerTime must be called explicitly",                           # 35
        "Expired session error: the client session token is not valid, "
        "FlowClient_RenewSession must be called explicitly"                                     # 36
    ]

    def __init__(self, message, error_code):
        """ Constructor for FlowCoreException class

        :param str message: error string
        :param int error_code: error_code set due to FlowCore API failure
        """
        connection_error = False
        if error_code:
            message = "{} caused by error code {}: {}".\
                format(message, error_code, FlowCoreException.__get_error_string(error_code))
            # TODO remove error_code == 1 check once bug in the flow library gets fixed
            connection_error = error_code in [1, 6, 12]

        super(FlowCoreException, self).__init__(message, error_code, connection_error)

    @classmethod
    def __get_error_string(cls, error_code):
        """ Get error message from error code

        :param int error_code: error_code set due to FlowCore API failure
        :return: error message
        :rtype: str
        """
        try:
            return cls.__flow_core_error_messages[error_code]

        except IndexError:
            return "Error msg for code: {} is not available".format(error_code)


class FlowMessagingException(FlowException):
    """ FlowMessagingException handles the exceptions raised for FlowMessaging API

    """
    __flow_messaging_error_messages = [
        "No error",
        "Internal",                        # 1
        "Unauthorised",                    # 2
        "Not found",                       # 3
        "Timeout",                         # 4
        "Network",                         # 5
        "Method unavailable",              # 6
        "Device registration argument"     # 7
    ]

    def __init__(self, message, error_code):
        """ Constructor for FlowMessagingException class

        :param str message: error string
        :param int error_code: error_code set due to FlowMessaging API failure
        """
        connection_error = False
        if error_code:
            message = "{} caused by error code {}: {}".\
                format(message, error_code, FlowMessagingException.__get_error_string(error_code))
            connection_error = error_code in [4, 5]
        super(FlowMessagingException, self).__init__(message, error_code, connection_error)

    @classmethod
    def __get_error_string(cls, error_code):
        """ Get error message from error code

        :param int error_code: error_code set due to FlowMessaging API failure
        :return: error message
        :rtype: str
        """
        try:
            return cls.__flow_messaging_error_messages[error_code]

        except IndexError:
            return "Error msg for code: {} is not available".format(error_code)


class FlowMessage(object):
    """ FlowMessage contains the message object sent by user or device

    """
    def __init__(self, message):
        """ Initialize message object

        :param message: message object
        """
        self.__message = message

    def get_message_content(self):
        """ Get message content

        :return: message content
        :rtype: str
        :raises: ValueError
        """
        message_content = copy.deepcopy(libflow_interface.get_message_content(self.__message))
        return message_content


# TBD: need to make this singleton
class FlowUser(object):
    """ Class representing Flow User

    """
    def __init__(self, name, password):
        """ User login with given username and password

        :param str name: username
        :param str password: password
        :raises: ValueError, FlowCoreException
        """
        libflow_interface.user_login(name, password)
        self.__logged_in_user = libflow_interface.get_logged_in_user()

    def find_device(self, device_type):
        """ Search for a device from list of user owned devices according to device type
            and return first found device

        :param str device_type: type of required device e.g "Linux"
        :return: FlowDevice object
        :raises: ValueError, FlowCoreException
        """
        if not device_type:
            raise ValueError("Invalid function parameter")

        device = None
        flow_devices = libflow_interface.get_user_owned_devices(self.__logged_in_user)
        for flow_device in flow_devices:
            if libflow_interface.get_device_type(flow_device) == device_type:
                device = FlowDevice(flow_device)
                break
        if not device:
            raise FlowException("Device not found")
        return device

    def get_owned_devices(self):
        """ Get all user owned devices

        :return: list of FlowDevices
        :rtype: list
        :raises: ValueError, FlowCoreException
        """
        all_devices = []
        flow_devices = libflow_interface.get_user_owned_devices(self.__logged_in_user)
        for flow_device in flow_devices:
            all_devices.append(FlowDevice(flow_device))

        return all_devices

    def enable_message_reception(self, callback_function):
        """ Set callback function to be called on reception of response message

        callback function is responsible for getting message content and parsing it
        :param callback_function: function to be called on reception of response message
        callback signature: callback_function(message)
        """
        libflow_interface.set_message_reception_callback(callback_function)

    def send_message_to_device(self, device, message, message_expiry_time):
        """ Send message to FlowDevice

        :param device: FlowDevice object
        :param str message: message to be sent e.g command to device
        :param int message_expiry_time: Time in seconds until which server tries to send message to
                                        device
        :raises: ValueError, FlowCoreException
        """
        libflow_interface.send_message_to_device(device.get_device_id(), message,
                                                 message_expiry_time)

    def logout(self):
        """ Function to Logout user

        :raises: FlowCoreException
        """
        libflow_interface.user_logout()


class FlowDevice(object):
    """ Class representing Flow Device
    """
    def __init__(self, flow_device):
        self.__flow_device = flow_device

    def get_device_type(self):
        """ Get device type

        :return: device type
        :rtype: str
        :raises: ValueError, FlowCoreException
        """
        return libflow_interface.get_device_type(self.__flow_device)

    def get_setting(self, key):
        """ Get device settings from specified key

        :param str key: key value whose associated settings is required
        :return: device settings
        :rtype: str
        :raises: ValueError, FlowCoreException
        """
        return libflow_interface.get_device_settings(self.__flow_device, key)

    def save_setting(self, key, value):
        """ Save device settings which is in key-value format

        :param str key: Associated key to device settings
        :param str value: settings value
        :raises: ValueError, FlowCoreException
        """
        libflow_interface.save_device_settings(self.__flow_device, key, value)

    def get_device_name(self):
        """ Get device name

        :return: device name
        :rtype: str
        :raises: ValueError, FlowCoreException
        """
        return libflow_interface.get_device_name(self.__flow_device)

    def get_device_id(self):
        """ Get device id

        :return device id
        :rtype: str
        :raises: ValueError, FlowCoreException
        """
        return libflow_interface.get_device_id(self.__flow_device)
