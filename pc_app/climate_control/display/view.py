""" This module acts as display app View(MVC)

"""


import matplotlib
matplotlib.use("AGG")
matplotlib.rcParams["backend.qt4"] = "PySide"
from matplotlib.backends.backend_qt4agg import FigureCanvasQTAgg as figure_canvas
from matplotlib.dates import DateFormatter
from matplotlib.ticker import LinearLocator
import logging
from PySide.QtGui import QMainWindow
from PySide.QtCore import Qt, QObject, Signal
from common.loginview import LoginDialog
from common.model import MAX_LATENCY_VALUE
from .mainwindow import Ui_DisplayMainWindow
import common.ui.strings as UIStrings
from enum import Enum

LOGGER = logging.getLogger(__name__)

# constants
TIME_FMT = "%H:%M:%S"
DEGREE_CELSIUS = u"\N{DEGREE SIGN}" + "C"
MAX_TEMPERATURE = 40
MIN_TEMPERATURE = 15
MAX_HUMIDITY = 70
MIN_HUMIDITY = 30

FONT_SIZE = "15"
LEGEND_FONT_SIZE = "11"


class DeviceEnum(Enum):
    """ Enum class for devices available

    """
    controller = 1
    sensor = 2
    actuator = 3


class DisplayMainWindow(QMainWindow, Ui_DisplayMainWindow):
    """ Display main windows

    """
    close_signal = Signal()

    def __init__(self, version=None, parent=None):
        super(DisplayMainWindow, self).__init__(parent)
        self.setupUi(self)
        if version:
            self.setWindowTitle("{} v{}".format(self.windowTitle(), version))
        self.setAttribute(Qt.WA_DeleteOnClose)
        self.screen_label.setScaledContents(True)
        self.setFocus()
        self.temperature_axes = None
        self.temperature_canvas = None
        self.temperature_figure = None
        self.humidity_axes = None
        self.humidity_canvas = None
        self.humidity_figure = None
        self.latency_label.setToolTip(UIStrings.LATENCY_TOOLTIP)
        self.latency.setToolTip(UIStrings.LATENCY_TOOLTIP)

    def closeEvent(self, *dummy_args, **dummy_kargs):
        self.close_signal.emit()


class DisplayView(QObject):
    """ Acts as MVC view, controls the UI

    """
    close_signal = Signal()

    def __init__(self, version=None, parent=None):
        """ DisplayView constructor

        :param str version: version to be shown on window title
        """
        super(DisplayView, self).__init__(parent)
        self.__login_dialog = None
        self.__display_window = None
        self.__version = version

    def init_login_view(self, user_names):
        """ Initializes the login view

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

    def update_status_bar(self, message):
        """ Update the display view status bar with the specified message

        :param str message: message string
        """
        if self.__display_window:
            self.__display_window.statusbar.showMessage(message)

    def set_connection_status(self, connection_status, color):
        """ Update the connection status label with the specified message

        :param str connection_status: current connection status (Network Down/ Internet Down/ OK)
        :param str color: color of the connection status(red/green)
        """

        if self.__display_window:
            self.__display_window.connection_status.setText(connection_status)
            self.__display_window.connection_status.setStyleSheet("color: {}".format(color))
        elif self.__login_dialog:
            # Ignore connection status = OK
            if connection_status != UIStrings.OK:
                self.__login_dialog.login_connection_status.setText(connection_status)

    def init_display_view(self, setting):
        """ Initializes display window

        Currently showing only thresholds from the settings
        :param ControllerSetting setting: initial settings
        """
        self.__display_window = DisplayMainWindow(self.__version)
        self.__display_window.close_signal.connect(self.__close_action)
        self.update_settings(setting)

        # add graph plot
        self.__display_window.temperature_figure = matplotlib.figure.Figure(facecolor="white",
                                                                            edgecolor="black")
        self.__display_window.temperature_canvas = figure_canvas(self.__display_window.
                                                                 temperature_figure)
        self.__display_window.graph_layout.addWidget(self.__display_window.temperature_canvas)

        self.__display_window.humidity_figure = matplotlib.figure.Figure(facecolor="white",
                                                                         edgecolor="black")
        self.__display_window.humidity_canvas = figure_canvas(self.__display_window.humidity_figure)
        self.__display_window.humidity_graph_layout.addWidget(self.__display_window.humidity_canvas)
        self.__init_graph()

        # Show display window
        self.__display_window.show()

    def update_settings(self, setting):
        """ Update the thresholds on display page

        :param ControllerSetting setting: controller setting object
        """
        if setting:
            self.__display_window.temperature_threshold.\
                setText("{:.2f}".format(setting.temperature_threshold))
            self.__display_window.humidity_threshold.\
                setText("{:.2f}".format(setting.humidity_threshold))

    def plot_graph(self, temperature_data, humidity_data):
        """ Plot time vs temperature graph and time vs humidity graph

        :param dict temperature_data: key-time, value-temperature
        :param dict humidity_data: key- time, value- temperature
        """
        self.__display_window.temperature_value.\
            setText("{:.2f}".format(temperature_data[temperature_data.keys()[-1]]))
        self.__display_window.humidity_value.\
            setText("{:.2f}".format(humidity_data[humidity_data.keys()[-1]]))
        self.__init_graph()
        self.__update_graph(temperature_data, humidity_data)

    def __init_graph(self):
        """ Initializes graph parameters

        """
        # initialize temperature graph
        if self.__display_window.temperature_axes:
            self.__display_window.temperature_axes.clear()
        if self.__display_window.temperature_figure:
            self.__display_window.temperature_figure.clf()
        self.__display_window.temperature_axes = self.__display_window.temperature_figure.\
            add_subplot(1, 1, 1)
        self.__display_window.temperature_axes.grid(True)
        self.__display_window.temperature_axes.set_ylim(MIN_TEMPERATURE, MAX_TEMPERATURE)
        self.__display_window.temperature_axes.set_ylabel("Temperature (" + DEGREE_CELSIUS + ")",
                                                          fontsize=FONT_SIZE)
        self.__display_window.temperature_axes.tick_params(axis="both", which="major",
                                                           labelsize=FONT_SIZE)
        self.__display_window.temperature_canvas.draw()

        # initialize humidity graph
        if self.__display_window.humidity_axes:
            self.__display_window.humidity_axes.clear()
        if self.__display_window.humidity_figure:
            self.__display_window.humidity_figure.clf()
        self.__display_window.humidity_axes = self.__display_window.humidity_figure.\
            add_subplot(1, 1, 1)
        self.__display_window.humidity_axes.grid(True)
        self.__display_window.humidity_axes.set_ylim(MIN_HUMIDITY, MAX_HUMIDITY)
        self.__display_window.humidity_axes.set_ylabel("Humidity (%RH)", fontsize=FONT_SIZE)
        self.__display_window.humidity_axes.tick_params(axis="both", which="major",
                                                        labelsize=FONT_SIZE)
        self.__display_window.humidity_canvas.draw()

    def __update_graph(self, temperature_data, humidity_data):
        """ Update graph with latest measurements

        :param dict temperature_data: key-time, value-temperature
        """

        # Clipping out of range temperature values to min and max values
        for key, _ in temperature_data.items():
            if temperature_data[key] > MAX_TEMPERATURE:
                temperature_data[key] = MAX_TEMPERATURE
            elif temperature_data[key] < MIN_TEMPERATURE:
                temperature_data[key] = MIN_TEMPERATURE

        # Clipping out of range humidity values to min and max values
        for key, _ in humidity_data.items():
            if humidity_data[key] > MAX_HUMIDITY:
                humidity_data[key] = MAX_HUMIDITY
            elif humidity_data[key] < MIN_HUMIDITY:
                humidity_data[key] = MIN_HUMIDITY

        # update temperature graph
        x_axis_data = temperature_data.keys()
        y_axis_data = temperature_data.values()
        self.__display_window.temperature_axes.plot(x_axis_data, y_axis_data, lw=3.0,
                                                    label="Temperature", color="blue")

        if self.__display_window.temperature_threshold.text():
            temperature_threshold = self.__display_window.temperature_threshold.text()
            y_axis_data = [temperature_threshold]*len(temperature_data.keys())
            self.__display_window.temperature_axes.\
                plot(x_axis_data, y_axis_data, color="red", linestyle="dashed",
                     label="Temperature Threshold", lw=3.0)

        self.__display_window.temperature_axes.legend(bbox_to_anchor=(0, 1), numpoints=1,
                                                      loc="lower left", fontsize=LEGEND_FONT_SIZE,
                                                      borderaxespad=0)

        self.__display_window.temperature_figure.autofmt_xdate()
        self.__display_window.temperature_axes.xaxis.set_major_formatter(DateFormatter(TIME_FMT))
        self.__display_window.temperature_axes.xaxis.set_major_locator(LinearLocator())

        self.__display_window.temperature_canvas.draw()

        # update humidity graph
        self.__display_window.humidity_axes.plot(humidity_data.keys(), humidity_data.values(),
                                                 lw=3.0, label="Humidity", color="green")

        if self.__display_window.humidity_threshold.text():
            humidity_threshold = self.__display_window.humidity_threshold.text()
            y_axis_data = [humidity_threshold]*len(humidity_data.keys())
            self.__display_window.humidity_axes.plot(humidity_data.keys(), y_axis_data,
                                                     color="red", linestyle="dashed",
                                                     label="Humidity Threshold", lw=3.0)

        self.__display_window.humidity_axes.legend(bbox_to_anchor=(0, 1), numpoints=1,
                                                   loc="lower left", fontsize=LEGEND_FONT_SIZE,
                                                   borderaxespad=0)

        self.__display_window.humidity_figure.autofmt_xdate()
        self.__display_window.humidity_axes.xaxis.set_major_formatter(DateFormatter(TIME_FMT))
        self.__display_window.humidity_axes.xaxis.set_major_locator(LinearLocator())
        self.__display_window.humidity_canvas.draw()

    def __update_device_status_label(self, widget, device_on):
        """ Update device status label on screen

        :param QLabel widget: pyside QLabel widget for text display
        :param bool device_on: True if device is "ONLINE"
        """
        if device_on:
            widget.setText(UIStrings.DEVICE_ONLINE)
            widget.setStyleSheet('color: green')
        else:
            widget.setText(UIStrings.DEVICE_OFFLINE)
            widget.setStyleSheet('color: red')

    def update_device_status(self, device_type, device_on):
        """ Update device status

        :param DeviceEnum device_type: device type e.g sensor or controller
        :param bool device_on: True if device is "ONLINE"
        """
        if self.__display_window:
            if device_type == DeviceEnum.controller:
                self.__update_device_status_label(self.__display_window.controller_status,
                                                  device_on)

            elif device_type == DeviceEnum.sensor:
                self.__update_device_status_label(self.__display_window.sensor_status, device_on)

            elif device_type == DeviceEnum.actuator:
                self.__update_device_status_label(self.__display_window.actuator_status, device_on)

            else:
                LOGGER.error("{} device doesn't exist".format(device_type))

    def update_relay_status(self, event):
        """ Update relay information on display like status, mode etc

        :param RelayStatusEvent event: RelayStatusEvent object
        """
        self.__display_window.heater_status.setText(UIStrings.RELAY_ON if event.relay_1_on else
                                                    UIStrings.RELAY_OFF)
        self.__display_window.fan_status.setText(UIStrings.RELAY_ON if event.relay_2_on else
                                                 UIStrings.RELAY_OFF)

        self.__display_window.heater_mode.setText(
            UIStrings.RELAY_AUTO if event.relay_1_mode == "AUTO" else UIStrings.RELAY_MANUAL)

        self.__display_window.fan_mode.setText(
            UIStrings.RELAY_AUTO if event.relay_2_mode == "AUTO" else UIStrings.RELAY_MANUAL)

    def update_latency(self, latency):
        """ Update latency information on display screen

        :param float latency: latency value, if None set latency as "NA"
        """
        if self.__display_window:
            if latency:
                if latency > MAX_LATENCY_VALUE:
                    # Avoid showing this value as it is likely to be incorrect latency value
                    self.__display_window.latency.setText("> {} sec".format(MAX_LATENCY_VALUE))
                else:
                    self.__display_window.latency.setText("{:.2f} sec".format(latency))
            else:
                self.__display_window.latency.setText("NA")

    def __close_action(self):
        """ Slot for display or login view close action

        """
        self.close_signal.emit()
