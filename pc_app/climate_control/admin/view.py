""" This module acts as admin app View(MVC)

"""

import logging
from enum import Enum
from PySide.QtGui import QMainWindow
from PySide.QtCore import Qt, QObject, Slot, Signal
from common.loginview import LoginDialog
from .mainwindow import Ui_AdminMainWindow
from common.message_parse import ControllerCommandEnum
from common.model import ControllerSetting
from common.ui import strings as UIStrings

LOGGER = logging.getLogger(__name__)


class RelayDevicesEnum(Enum):
    """ Enum class for types of device connected to relay

    """
    heater = 1
    fan = 2


# pylint: disable=too-many-public-methods
class AdminMainWindow(QMainWindow, Ui_AdminMainWindow):
    """ Admin main windows

    Shows controller device setting and allows user to modify it
    """
    close_signal = Signal()

    def __init__(self, version=None, parent=None):
        super(AdminMainWindow, self).__init__(parent)
        self.setupUi(self)
        # Using degree sign in QtCreator generates some invalid ascii values which diff complains
        # so setting it here
        self.temperature_label.setText("Temperature(" + u"\N{DEGREE SIGN}" + "C)")
        if version:
            self.setWindowTitle("{} v{}".format(self.windowTitle(), version))
        self.setAttribute(Qt.WA_DeleteOnClose)
        self.screen_label.setScaledContents(True)
        self.setFocus()

    def closeEvent(self, *dummy_args, **dummy_kargs):
        self.close_signal.emit()


class AdminView(QObject):
    """ Acts as MVC view, controls the UI

    """
    close_signal = Signal()
    __combo_box_above_index = 0
    __combo_box_below_index = 1

    def __init__(self, version=None, parent=None):
        """ AdminView constructor

        :param str version: version to be shown on window title
        """
        super(AdminView, self).__init__(parent)
        self.__login_dialog = None
        self.__admin_window = None
        self.__save_setting_callback = None
        self.__version = version

    def init_login_view(self, user_names):
        """ Initializes the login view

        :param list user_names: list of user name strings, will be used for showing auto-completion
        """
        self.__login_dialog = LoginDialog(user_names, self.__version)
        self.__login_dialog.close_signal.connect(self.__close_action)
        self.__login_dialog.show()

    def register_login_callback(self, callback):
        """ Register login button callback

        :param callback: Slot function to be called on button click
                         callback signature: callback(user_name:str, password:str)
        """
        if self.__login_dialog:
            self.__login_dialog.login.clicked.connect(
                lambda: callback(str(self.__login_dialog.user_name.text()),
                                 str(self.__login_dialog.password.text())))

    def __save_setting_clicked(self):
        """ Local function called when save setting is clicked

        Creates ControllerSetting object and calls controller callback
        """
        setting = {"temperature_threshold": self.__admin_window.temperature_threshold.value(),
                   "humidity_threshold": self.__admin_window.humidity_threshold.value(),
                   "temperature_read_interval":
                   self.__admin_window.temperature_read_interval.value(),
                   "humidity_read_interval": self.__admin_window.humidity_read_interval.value(),
                   "temperature_read_delta": self.__admin_window.temperature_read_delta.value(),
                   "humidity_read_delta": self.__admin_window.humidity_read_delta.value(),
                   "sensor_heartbeat": self.__admin_window.sensor_heartbeat.value(),
                   "actuator_heartbeat": self.__admin_window.actuator_heartbeat.value(),
                   "controller_heartbeat": self.__admin_window.controller_heartbeat.value()}
        if self.__admin_window.temperature_orientation.currentIndex() == \
                AdminView.__combo_box_above_index:
            setting["temperature_orientation"] = ControllerSetting.threshold_orientation_above
        else:
            setting["temperature_orientation"] = ControllerSetting.threshold_orientation_below

        if self.__admin_window.humidity_orientation.currentIndex() == \
                AdminView.__combo_box_above_index:
            setting["humidity_orientation"] = ControllerSetting.threshold_orientation_above
        else:
            setting["humidity_orientation"] = ControllerSetting.threshold_orientation_below
        self.__save_setting_callback(ControllerSetting(setting=setting))

    def register_save_setting_callback(self, callback):
        """ Register save button callback

        :param callback: Slot function to be called on button click
                         callback signature: callback(setting)
        """
        if self.__admin_window:
            self.__save_setting_callback = callback
            self.__admin_window.save_settings_btn.clicked.connect(self.__save_setting_clicked)

    def register_control_callback(self, callback):
        """ Register callback for the manual control buttons

        :param callback: Slot function to be called on button click
                         callback signature: callback(command)
        """
        if self.__admin_window:
            self.__admin_window.heater_on_btn.clicked.\
                connect(lambda: callback(ControllerCommandEnum.relay_1_on))
            self.__admin_window.heater_off_btn.clicked.\
                connect(lambda: callback(ControllerCommandEnum.relay_1_off))
            self.__admin_window.heater_automatic_btn.clicked.\
                connect(lambda: callback(ControllerCommandEnum.relay_1_auto))
            self.__admin_window.fan_on_btn.clicked.\
                connect(lambda: callback(ControllerCommandEnum.relay_2_on))
            self.__admin_window.fan_off_btn.clicked.\
                connect(lambda: callback(ControllerCommandEnum.relay_2_off))
            self.__admin_window.fan_automatic_btn.clicked.\
                connect(lambda: callback(ControllerCommandEnum.relay_2_auto))

    def login_view_show_error(self, message=None):
        """ Show error message on login screen

        :param str message: Optional string, clears the status if None else sets it
        """
        if self.__login_dialog:
            if message:
                self.__login_dialog.login_status.setText(message)
            else:
                self.__login_dialog.login_status.clear()

    def close_login_view(self):
        """ Hides the login view

        """
        if self.__login_dialog:
            self.__login_dialog.hide()
            self.__login_dialog.deleteLater()

    def init_admin_view(self, setting):
        """ Initializes the admin view

        :param ControllerSetting setting: initial setting
        """
        self.__admin_window = AdminMainWindow(self.__version)
        self.update_settings(setting)
        self.__admin_window.close_signal.connect(self.__close_action)
        self.__admin_window.show()

    def update_settings(self, setting):
        """ Update the thresholds on admin page spin box

        :param ControllerSetting setting: controller setting object
        """
        if setting:
            self.__admin_window.temperature_threshold.setValue(setting.temperature_threshold)
            self.__admin_window.humidity_threshold.setValue(setting.humidity_threshold)
            self.__admin_window.temperature_read_interval.\
                setValue(setting.temperature_read_interval)
            self.__admin_window.humidity_read_interval.setValue(setting.humidity_read_interval)
            self.__admin_window.temperature_read_delta.setValue(setting.temperature_read_delta)
            self.__admin_window.humidity_read_delta.setValue(setting.humidity_read_delta)
            self.__admin_window.sensor_heartbeat.setValue(setting.sensor_heartbeat)
            self.__admin_window.actuator_heartbeat.setValue(setting.actuator_heartbeat)
            self.__admin_window.controller_heartbeat.setValue(setting.controller_heartbeat)
            if setting.temperature_orientation == ControllerSetting.threshold_orientation_above:
                self.__admin_window.temperature_orientation.setCurrentIndex(
                    AdminView.__combo_box_above_index)
            else:
                self.__admin_window.temperature_orientation.setCurrentIndex(
                    AdminView.__combo_box_below_index)
            if setting.humidity_orientation == ControllerSetting.threshold_orientation_above:
                self.__admin_window.humidity_orientation.setCurrentIndex(
                    AdminView.__combo_box_above_index)
            else:
                self.__admin_window.humidity_orientation.setCurrentIndex(
                    AdminView.__combo_box_below_index)

    def __set_button_underline(self, button, enable):
        """ If enable is true, sets button text underline on; otherwise sets underline off.

        :param QPushButton button: button widget
        :param bool enable: whether to underline it
        """
        font = button.font()
        font.setUnderline(enable)
        button.setFont(font)

    def update_manual_buttons_status(self, relay_type, relay_status, relay_mode):
        """ Underlines the appropriate button based on status and mode

        :param RelayDevicesEnum relay_type: which relay status to set, heater or fan
        :param bool relay_status: current status on or off
        :param str relay_mode: current mode "AUTO" or "MANUAL"
        """
        if self.__admin_window:
            button = {}
            if relay_type == RelayDevicesEnum.heater:
                button = {"auto": self.__admin_window.heater_automatic_btn,
                          "on": self.__admin_window.heater_on_btn,
                          "off": self.__admin_window.heater_off_btn}
            elif relay_type == RelayDevicesEnum.fan:
                button = {"auto": self.__admin_window.fan_automatic_btn,
                          "on": self.__admin_window.fan_on_btn,
                          "off": self.__admin_window.fan_off_btn}
            else:
                LOGGER.error("Invalid relay type")

            if button:
                # clear the previous underlines and set again based on status, mode
                self.__set_button_underline(button["auto"], False)
                self.__set_button_underline(button["on"], False)
                self.__set_button_underline(button["off"], False)
                if relay_mode == "AUTO":
                    self.__set_button_underline(button["auto"], True)
                elif relay_status:
                    self.__set_button_underline(button["on"], True)
                else:
                    self.__set_button_underline(button["off"], True)

    def update_status_bar(self, message):
        """ Update the admin view status bar with the specified message

        :param str message: message string
        """
        if self.__admin_window:
            self.__admin_window.statusbar.showMessage(message)

    def set_connection_status(self, connection_status, color):
        """ Update the connection status label with the specified message

        :param str connection_status: current connection status (Network Down/ Internet Down/ OK)
        :param str color: color of the connection status(red/green)
        """
        if self.__admin_window:
            self.__admin_window.connection_status.setText(connection_status)
            self.__admin_window.connection_status.setStyleSheet('color: {}'.format(color))
        elif self.__login_dialog:
            # Ignore connection status = OK
            if connection_status != UIStrings.OK:
                self.__login_dialog.login_connection_status.setText(connection_status)

    @Slot()
    def __close_action(self):
        """ Slot for admin or login view close action

        """
        self.close_signal.emit()

# pylint: enable=too-many-public-methods
