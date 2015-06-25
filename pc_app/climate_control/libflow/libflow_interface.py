""" libflow_interface.py is a wrapper library used to provide interface for the python
    application to access libflow services

"""
import copy
import libflowcore as flow_core
import libflowmessaging as flow_messaging
from libflow.flow import FlowCoreException, FlowMessagingException


def set_nvs_file(file_path):
    """ Sets flow nvs file path

    :param str file_path: nvs file path
    :raises: ValueError
    """
    if file_path:
        flow_core.FlowNVS_SetLocation(file_path)
    else:
        raise ValueError("Invalid function parameter")


def get_memory_manager():
    """ Get memory manager

    :return: memory_manager object
    :raises: FlowCoreException
    """
    memory_manager = flow_core.FlowMemoryManager_New()
    if not memory_manager:
        raise FlowCoreException("Failed to allocate memory", flow_core.Flow_GetLastError())
    return memory_manager


def get_new_device(memory_manager):
    """ Create new device from memory manager

    :param memory_manager: memory manager object
    :return: Flow device object
    :rtype: FlowDevice
    :raises: ValueError, FlowCoreException
    """
    if not memory_manager:
        raise ValueError("Invalid function parameter")

    device = flow_core.FlowDevice_New(memory_manager)
    if not device:
        raise FlowCoreException("Failed to create new device", flow_core.Flow_GetLastError())

    return device


def initialize_flow():
    """ Initialize LibFlowCore and LibFlowMessaging

    :raises: FlowCoreException
    """
    flow_core.FlowCore_InitialiseLibOnly()
    flow_core.FlowCore_RegisterTypes()
    flow_messaging.FlowMessaging_RegisterTypes()
    if not flow_messaging.FlowMessaging_Initialise():
        raise FlowMessagingException("Failed to initialise messaging",
                                     flow_messaging.FlowThread_GetLastError())


def connect_to_flow_server(url, key, secret):
    """ Connect to flow cloud

    :param str url: flow server url
    :param str key: flow server key
    :param str secret: flow server secret
    :raises: ValueError, FlowCoreException
    """
    if not (url and key and secret):
        raise ValueError("Invalid function parameters")

    if not flow_core.FlowClient_ConnectToServer(url, key, secret, False):
        raise FlowCoreException("Connection to Flow Server Failed", flow_core.Flow_GetLastError())


def user_login(username, password):
    """ Login as User

    :param str username: username
    :param str password: password
    :raises: ValueError, FlowCoreException
    """
    if not (username and password):
        raise ValueError("Invalid function parameters")

    if not flow_core.FlowClient_LoginAsUser(username, password, False):
        raise FlowCoreException("Username or password incorrect", flow_core.Flow_GetLastError())


def get_logged_in_user():
    """ Get logged-in user object

    :return: logged in user object
    :raises: FlowCoreException
    """
    memory_manager = get_memory_manager()
    try:
        user = flow_core.FlowClient_GetLoggedInUser(memory_manager)
        if not user:
            raise FlowCoreException("Failed to get logged-in user", flow_core.Flow_GetLastError())

        cached_user = flow_core.FlowUser_New(None)
        if not cached_user:
            raise FlowCoreException("Failed to create new user", flow_core.Flow_GetLastError())

        flow_core.FlowUser_CopyFrom(cached_user, user)

    finally:
        flow_core.FlowClearMemory(memory_manager)

    return cached_user


def get_user_owned_devices(logged_in_user):
    """ Get devices owned by user

    :param: loggedInUser
    :return: user devices
    :rtype: list
    :raises: ValueError, FlowCoreException
    """
    all_devices = []
    if not logged_in_user:
        raise ValueError("Invalid function parameters")

    memory_manager = get_memory_manager()
    try:
        cached_user = flow_core.FlowUser_New(memory_manager)
        if not cached_user:
            raise FlowCoreException("Failed to create new user", flow_core.Flow_GetLastError())

        flow_core.FlowUser_CopyFrom(cached_user, logged_in_user)
        my_devices = flow_core.FlowUser_RetrieveOwnedDevices(cached_user,
                                                             flow_core.FLOW_DEFAULT_PAGE_SIZE)
        if not my_devices:
            raise FlowCoreException("User has no devices", flow_core.Flow_GetLastError())

        for index in range(flow_core.FlowDevices_GetCount(my_devices)):
            device = flow_core.FlowDevices_GetItem(my_devices, index)
            if not device:
                raise FlowCoreException("Failed to get a device from device list",
                                        flow_core.Flow_GetLastError())
            cached_device = flow_core.FlowDevice_New(None)
            if not cached_device:
                raise FlowCoreException("Failed to create new device",
                                        flow_core.Flow_GetLastError())
            flow_core.FlowDevice_CopyFrom(cached_device, device)
            all_devices.append(cached_device)

    finally:
        flow_core.FlowClearMemory(memory_manager)

    return all_devices


def get_device_name(device):
    """ Get name of device

    :param FLowDevice device: device object
    :return: device name
    :rtype: str
    :raises: ValueError, FlowCoreException
    """
    if not device:
        raise ValueError("Invalid function parameter")

    memory_manager = get_memory_manager()
    try:
        cached_device = get_new_device(memory_manager)
        flow_core.FlowDevice_CopyFrom(cached_device, device)
        cached_device_name = copy.deepcopy(flow_core.FlowDevice_GetDeviceName(cached_device))

    finally:
        flow_core.FlowClearMemory(memory_manager)

    return cached_device_name


def get_device_id(device):
    """ Get ID of device

    :param FlowDevice device: flow device object
    :return: device ID
    :rtype: str
    :raises: ValueError, FlowCoreException
    """
    if not device:
        raise ValueError("Invalid function parameter")

    memory_manager = get_memory_manager()
    try:
        cached_device = get_new_device(memory_manager)
        flow_core.FlowDevice_CopyFrom(cached_device, device)
        cached_device_id = copy.deepcopy(flow_core.FlowDevice_GetDeviceID(cached_device))

    finally:
        flow_core.FlowClearMemory(memory_manager)

    return cached_device_id


def get_device_type(device):
    """ Get Type of device

    :param FlowDevice device: flow device object
    :return: device Type
    :rtype: str
    :raises: ValueError, FlowCoreException
    """
    if not device:
        raise ValueError("Invalid function parameter")

    memory_manager = get_memory_manager()
    try:
        cached_device = get_new_device(memory_manager)
        flow_core.FlowDevice_CopyFrom(cached_device, device)
        cached_device_type = copy.deepcopy(flow_core.FlowDevice_GetDeviceType(cached_device))

    finally:
        flow_core.FlowClearMemory(memory_manager)

    return cached_device_type


def get_device_settings(device, key):
    """ Get device settings from flow cloud

    :param FlowDevice device: flow device object
    :param str key: key  parameter whose associated settings is to be retrieved
    :return: settings
    :rtype: str
    :raises: ValueError, FlowCoreException
    """
    if not (device and key):
        raise ValueError("Invalid function parameters")

    memory_manager = get_memory_manager()
    try:
        cached_device = get_new_device(memory_manager)
        if not cached_device:
            raise FlowCoreException("Failed to create new device", flow_core.Flow_GetLastError())

        flow_core.FlowDevice_CopyFrom(cached_device, device)
        if not flow_core.FlowDevice_CanRetrieveSettings(cached_device):
            raise FlowCoreException("Device does not have settings", flow_core.Flow_GetLastError())

        setting = flow_core.FlowDevice_RetrieveSetting(cached_device, key)
        if not setting:
            raise FlowCoreException("None settings retrieved", flow_core.Flow_GetLastError())

        cached_value = copy.deepcopy(flow_core.FlowSetting_GetValue(setting))

    finally:
        flow_core.FlowClearMemory(memory_manager)

    return cached_value


def save_device_settings(device, key, value):
    """ Save device settings to flow cloud which is in key-value format

    :param FlowDevice device: flow device object
    :param str key: Associated key to device settings
    :param str value: device settings value which is to be saved
    :raises: ValueError, FlowCoreException
    """
    if not (device and key and value):
        raise ValueError("Invalid function parameters")

    memory_manager = get_memory_manager()
    try:
        cached_device = get_new_device(memory_manager)
        if not cached_device:
            raise FlowCoreException("Failed to create new device", flow_core.Flow_GetLastError())

        flow_core.FlowDevice_CopyFrom(cached_device, device)
        if not flow_core.FlowDevice_CanRetrieveSettings(cached_device):
            raise FlowCoreException("Device does not have settings", flow_core.Flow_GetLastError())

        dev_settings = flow_core.FlowDevice_RetrieveSettings(cached_device,
                                                             flow_core.FLOW_DEFAULT_PAGE_SIZE)
        if not dev_settings:
            raise FlowCoreException("Failed to retrieve device settings",
                                    flow_core.Flow_GetLastError())

        flow_core.FlowSettings_SaveSetting(memory_manager, dev_settings, key, value)
        if flow_core.Flow_GetLastError() != flow_core.FlowError_NoError:
            raise FlowCoreException("Save settings failed", flow_core.Flow_GetLastError())

    finally:
        flow_core.FlowClearMemory(memory_manager)


def set_message_reception_callback(callback_function):
    """ Set callback function to be called on reception of response message

    :param callback_function: function to be called on reception of response message
                              callback signature: callback_function(message)
    """
    if not callback_function:
        raise ValueError("Invalid parameter")

    flow_messaging.Flow_SetMessageReceivedListenerForUser(callback_function)
    if flow_messaging.Flow_GetLastError() != flow_core.FlowError_NoError:
        raise FlowCoreException("Setting message reception callback failed",
                                flow_core.Flow_GetLastError())


def get_message_content(message):
    """ Retrieve the response message content

    :return: message content
    :rtype: str
    :raises: ValueError
    """
    if message:
        return flow_messaging.FlowMessagingMessage_GetContent(message)

    raise ValueError("Invalid function parameter")


def get_message_content_length(message):
    """ Retrieve the response message content length

    :return: message content length
    :rtype: int
    :raises: ValueError
    """
    if message:
        return flow_messaging.FlowMessagingMessage_GetContentLength(message)

    raise ValueError("Invalid function parameter")


def send_message_to_device(device_id, message, message_expiry_time):
    """ Send message to the device

    :param str device_id: Id of device to whom the message is to be sent
    :param str message: message content
    :param int message_expiry_time: Time in seconds until which server tries to send message to
                                    device
    :raises: ValueError, FlowMessagingException
    """
    if device_id and message:
        if not flow_messaging.FlowMessaging_SendMessageToDevice(device_id, "text/xml", message,
                                                                len(message), message_expiry_time):
            raise FlowMessagingException("Failed to send message",
                                         flow_messaging.Flow_GetLastError())
    else:
        raise ValueError("Invalid function parameters")


def user_logout():
    """ Used to logout the user

    :raises: FlowCoreException
    """
    flow_messaging.FlowClient_Logout()
    if flow_core.Flow_GetLastError() != flow_core.FlowError_NoError:
        raise FlowCoreException("Failed to logout", flow_core.Flow_GetLastError())


def shut_down():
    """ Shutdown FlowCore

    :raises: FlowCoreException
    """
    flow_core.FlowCore_Shutdown()
    if flow_core.Flow_GetLastError() != flow_core.FlowError_NoError:
        raise FlowCoreException("Shut down failed", flow_core.Flow_GetLastError())
