""" This module will act as MVC controller, it will sit between model and view

"""

import logging
from PySide.QtCore import QObject, Slot
from .view import AdminView, RelayDevicesEnum
import common.ui.strings as Strings
from common.model import ClimateControlModel, FlowLibraryError, LoginFailed, ControllerNotFound, \
    UserList, InvalidCommand, ControllerSetting
from common.message_parse import ControllerResponseEnum, ControllerCommandEnum, RelayStatusEvent
from common.miscellaneous import get_app_version


LOGGER = logging.getLogger(__name__)


class AdminController(QObject):
    """ Controller class for the admin app
    Acts as a coordinator between ClimateControlModel and AdminView
    """
    def __init__(self, parent=None):
        """ Creates View and shows the login screen

        """
        super(AdminController, self).__init__(parent)
        _, external_version = get_app_version()
        self.__admin_app = AdminView(external_version)
        self.__admin_app.init_login_view(UserList().user_names)
        self.__admin_app.register_login_callback(self.__login_btn_clicked)
        self.__admin_app.close_signal.connect(self.__close)
        self.__admin_view_initialized = False
        self.__model = None

    @Slot(str, str)
    def __login_btn_clicked(self, user_name, password):
        """ Slot for the login button

         Tries to create model with the provided user_name and password
        :param str user_name: flow user name
        :param str password: flow user password
        """
        # clear any old status
        self.__admin_app.login_view_show_error(None)
        LOGGER.debug("Trying to login with username = {}".format(user_name))

        try:
            # try to initialize model
            self.__model = ClimateControlModel()
            self.__model.connection_status.connect(self.__connection_status_result)
            self.__model.initialize(user_name, password)
            self.__model.save_setting_result.connect(self.__save_setting_result)
            self.__model.command_sending_result.connect(self.__command_sending_result)
            self.__model.command_response_received.connect(self.__command_response_received)
            self.__model.relay_status_changed.connect(self.__relay_status_changed)

            # Save username if no exception comes
            UserList().add_name(user_name)
            # login pass, try to get settings
            self.__model.setting_received.connect(self.__get_setting_response)
            self.__model.get_settings()

        except LoginFailed:
            self.__admin_app.login_view_show_error(Strings.LOGIN_FAILED)
            LOGGER.exception("Login failed")
        except ControllerNotFound:
            self.__admin_app.login_view_show_error(Strings.CONTROLLER_DEVICE_NOT_FOUND)
            LOGGER.exception("Controller device not found")
        except FlowLibraryError as error:
            self.__admin_app.login_view_show_error(Strings.FLOW_LIBRARY_ERROR)
            LOGGER.exception(error.message)

    @Slot(ControllerSetting)
    def __save_setting_btn_clicked(self, setting):
        """ Slot for the save settings button

        Asks model to save the data
        :param ControllerSetting setting: setting object contains various parameter from the view
        """
        self.__admin_app.update_status_bar(None)
        self.__model.save_settings(setting)

    @Slot(ControllerCommandEnum)
    def __manual_control_btn_clicked(self, command):
        """ Slot called when manual control buttons are clicked

        Sends corresponding command to the controller
        :param ControllerCommandEnum command: command to be sent to controller
        """
        # clear previous status message
        self.__admin_app.update_status_bar(None)
        try:
            self.__model.send_command(command)
        except InvalidCommand as error:
            LOGGER.exception(error)

    @Slot(dict)
    def __get_setting_response(self, result):
        """ Slot for the get setting response from ClimateControlModel

        Initializes admin view with received setting or shows error
        :param dict result: dictionary of the setting to be shown on configuration tab
        """
        if not self.__admin_view_initialized:
            self.__admin_app.close_login_view()
            self.__admin_app.init_admin_view(result["setting"])
            self.__admin_app.register_save_setting_callback(self.__save_setting_btn_clicked)
            self.__admin_app.register_control_callback(self.__manual_control_btn_clicked)
            self.__admin_view_initialized = True
            if result["error"]:
                self.__admin_app.update_status_bar(Strings.SETTING_NOT_RECEIVED)
                LOGGER.error(result["error"])
        else:
            self.__admin_app.update_settings(result["setting"])

    @Slot(dict)
    def __save_setting_result(self, result):
        """ Slot function for save setting result

        Update the UI with the result
        :param dict result: result string is set if any error in saving setting
        """
        if result["error"]:
            self.__admin_app.update_status_bar(Strings.SAVE_SETTING_FAILURE)

    @Slot(dict)
    def __command_sending_result(self, result):
        """ Slot function for command sending result

        :param dict result: "error" set if any command sending failed
        """
        if result["error"]:
            self.__admin_app.update_status_bar(Strings.MESSAGE_SENDING_FAILED)

    @Slot(ControllerResponseEnum)
    def __command_response_received(self, response):
        """ Called when model receives a response message

        :param dict response: response enum
        """
        # translate the responses to UI strings
        ui_strings = {ControllerResponseEnum.relay_1_on: Strings.HEATER_ON,
                      ControllerResponseEnum.relay_1_off: Strings.HEATER_OFF,
                      ControllerResponseEnum.relay_1_auto: Strings.HEATER_AUTO,
                      ControllerResponseEnum.relay_2_on: Strings.FAN_ON,
                      ControllerResponseEnum.relay_2_off: Strings.FAN_OFF,
                      ControllerResponseEnum.relay_2_auto: Strings.FAN_AUTO,
                      ControllerResponseEnum.retrieve_settings_success:
                          Strings.SAVE_SETTING_SUCCESS,
                      ControllerResponseEnum.retrieve_settings_failure.value:
                          Strings.SAVE_SETTING_FAILURE}
        try:
            self.__admin_app.update_status_bar(ui_strings[response["response"]])
        except KeyError:
            LOGGER.exception("Cannot find string for the response {}".format(response["response"]))

    @Slot(dict)
    def __connection_status_result(self, connection_status):
        """Slot function for connection status result

        :param dict connection_status: dictionary containing network and internet status
        """
        status = Strings.OK
        color = "green"
        if not connection_status["network"]:
            color = "red"
            status = Strings.NETWORK_DOWN
        elif not connection_status["internet"]:
            status = Strings.INTERNET_DOWN
            color = "red"

        self.__admin_app.set_connection_status(status, color)

    @Slot(RelayStatusEvent)
    def __relay_status_changed(self, event):
        """ Slot called when there is change in relay status or mode

        :param RelayStatusEvent event: relay status event object
        """
        if self.__admin_app:
            # relay 1 is connected to heater and relay 2 is connected to fan
            self.__admin_app.update_manual_buttons_status(RelayDevicesEnum.heater,
                                                          event.relay_1_on, event.relay_1_mode)
            self.__admin_app.update_manual_buttons_status(RelayDevicesEnum.fan,
                                                          event.relay_2_on, event.relay_2_mode)

    @Slot()
    def __close(self):
        """ Slot function when view gets closed

        Tries to de-initialize the model
        """
        LOGGER.debug("closing model")
        if self.__model:
            self.__model.close()
