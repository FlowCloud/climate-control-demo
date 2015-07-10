# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'climate_control/admin/mainwindow.ui'
#
# Created: Thu Jun 25 16:29:03 2015
#      by: pyside-uic 0.2.15 running on PySide 1.2.1
#
# WARNING! All changes made in this file will be lost!

from PySide import QtCore, QtGui

class Ui_AdminMainWindow(object):
    def setupUi(self, AdminMainWindow):
        AdminMainWindow.setObjectName("AdminMainWindow")
        AdminMainWindow.resize(1024, 700)
        font = QtGui.QFont()
        font.setPointSize(15)
        AdminMainWindow.setFont(font)
        icon = QtGui.QIcon()
        icon.addPixmap(QtGui.QPixmap(":/images/icon.png"), QtGui.QIcon.Normal, QtGui.QIcon.Off)
        AdminMainWindow.setWindowIcon(icon)
        self.centralwidget = QtGui.QWidget(AdminMainWindow)
        self.centralwidget.setObjectName("centralwidget")
        self.screen_label = QtGui.QLabel(self.centralwidget)
        self.screen_label.setGeometry(QtCore.QRect(0, -2, 1024, 80))
        self.screen_label.setMinimumSize(QtCore.QSize(0, 80))
        self.screen_label.setText("")
        self.screen_label.setPixmap(QtGui.QPixmap(":/images/banner.png"))
        self.screen_label.setObjectName("screen_label")
        self.layoutWidget = QtGui.QWidget(self.centralwidget)
        self.layoutWidget.setGeometry(QtCore.QRect(10, 87, 1001, 581))
        self.layoutWidget.setObjectName("layoutWidget")
        self.verticalLayout_4 = QtGui.QVBoxLayout(self.layoutWidget)
        self.verticalLayout_4.setContentsMargins(0, 0, 0, 0)
        self.verticalLayout_4.setObjectName("verticalLayout_4")
        self.horizontalLayout_3 = QtGui.QHBoxLayout()
        self.horizontalLayout_3.setObjectName("horizontalLayout_3")
        spacerItem = QtGui.QSpacerItem(40, 20, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum)
        self.horizontalLayout_3.addItem(spacerItem)
        self.connection_status_label = QtGui.QLabel(self.layoutWidget)
        self.connection_status_label.setEnabled(True)
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Fixed, QtGui.QSizePolicy.Preferred)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.connection_status_label.sizePolicy().hasHeightForWidth())
        self.connection_status_label.setSizePolicy(sizePolicy)
        self.connection_status_label.setMinimumSize(QtCore.QSize(0, 22))
        font = QtGui.QFont()
        font.setWeight(75)
        font.setBold(True)
        self.connection_status_label.setFont(font)
        self.connection_status_label.setAlignment(QtCore.Qt.AlignLeading|QtCore.Qt.AlignLeft|QtCore.Qt.AlignVCenter)
        self.connection_status_label.setObjectName("connection_status_label")
        self.horizontalLayout_3.addWidget(self.connection_status_label)
        self.connection_status = QtGui.QLabel(self.layoutWidget)
        self.connection_status.setEnabled(True)
        self.connection_status.setMaximumSize(QtCore.QSize(16777215, 16777215))
        font = QtGui.QFont()
        font.setPointSize(15)
        self.connection_status.setFont(font)
        self.connection_status.setStyleSheet("color: rgb(42, 126, 70);")
        self.connection_status.setScaledContents(True)
        self.connection_status.setObjectName("connection_status")
        self.horizontalLayout_3.addWidget(self.connection_status)
        spacerItem1 = QtGui.QSpacerItem(40, 20, QtGui.QSizePolicy.Fixed, QtGui.QSizePolicy.Minimum)
        self.horizontalLayout_3.addItem(spacerItem1)
        self.verticalLayout_4.addLayout(self.horizontalLayout_3)
        spacerItem2 = QtGui.QSpacerItem(20, 16, QtGui.QSizePolicy.Minimum, QtGui.QSizePolicy.Expanding)
        self.verticalLayout_4.addItem(spacerItem2)
        self.frame = QtGui.QFrame(self.layoutWidget)
        self.frame.setFrameShape(QtGui.QFrame.Box)
        self.frame.setObjectName("frame")
        self.gridLayout = QtGui.QGridLayout(self.frame)
        self.gridLayout.setContentsMargins(9, -1, -1, -1)
        self.gridLayout.setHorizontalSpacing(10)
        self.gridLayout.setVerticalSpacing(9)
        self.gridLayout.setObjectName("gridLayout")
        self.humidity_read_interval = QtGui.QSpinBox(self.frame)
        self.humidity_read_interval.setEnabled(True)
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Preferred, QtGui.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.humidity_read_interval.sizePolicy().hasHeightForWidth())
        self.humidity_read_interval.setSizePolicy(sizePolicy)
        self.humidity_read_interval.setMinimumSize(QtCore.QSize(0, 22))
        self.humidity_read_interval.setMinimum(100)
        self.humidity_read_interval.setMaximum(100000)
        self.humidity_read_interval.setSingleStep(100)
        self.humidity_read_interval.setProperty("value", 2500)
        self.humidity_read_interval.setObjectName("humidity_read_interval")
        self.gridLayout.addWidget(self.humidity_read_interval, 3, 5, 1, 1)
        self.sensor_label = QtGui.QLabel(self.frame)
        self.sensor_label.setEnabled(True)
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Fixed, QtGui.QSizePolicy.Preferred)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.sensor_label.sizePolicy().hasHeightForWidth())
        self.sensor_label.setSizePolicy(sizePolicy)
        self.sensor_label.setMinimumSize(QtCore.QSize(0, 22))
        font = QtGui.QFont()
        font.setWeight(75)
        font.setBold(True)
        self.sensor_label.setFont(font)
        self.sensor_label.setAlignment(QtCore.Qt.AlignLeading|QtCore.Qt.AlignLeft|QtCore.Qt.AlignVCenter)
        self.sensor_label.setObjectName("sensor_label")
        self.gridLayout.addWidget(self.sensor_label, 0, 1, 1, 1)
        self.read_delta_label = QtGui.QLabel(self.frame)
        self.read_delta_label.setEnabled(True)
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Fixed, QtGui.QSizePolicy.Preferred)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.read_delta_label.sizePolicy().hasHeightForWidth())
        self.read_delta_label.setSizePolicy(sizePolicy)
        self.read_delta_label.setMinimumSize(QtCore.QSize(0, 22))
        font = QtGui.QFont()
        font.setWeight(75)
        font.setBold(True)
        self.read_delta_label.setFont(font)
        self.read_delta_label.setAlignment(QtCore.Qt.AlignLeading|QtCore.Qt.AlignLeft|QtCore.Qt.AlignVCenter)
        self.read_delta_label.setWordWrap(False)
        self.read_delta_label.setObjectName("read_delta_label")
        self.gridLayout.addWidget(self.read_delta_label, 0, 4, 1, 1)
        self.temperature_read_delta = QtGui.QDoubleSpinBox(self.frame)
        self.temperature_read_delta.setEnabled(True)
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Preferred, QtGui.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.temperature_read_delta.sizePolicy().hasHeightForWidth())
        self.temperature_read_delta.setSizePolicy(sizePolicy)
        self.temperature_read_delta.setMinimumSize(QtCore.QSize(0, 22))
        self.temperature_read_delta.setSingleStep(0.5)
        self.temperature_read_delta.setProperty("value", 0.5)
        self.temperature_read_delta.setObjectName("temperature_read_delta")
        self.gridLayout.addWidget(self.temperature_read_delta, 2, 4, 1, 1)
        self.threshold_label = QtGui.QLabel(self.frame)
        self.threshold_label.setEnabled(True)
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Fixed, QtGui.QSizePolicy.Preferred)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.threshold_label.sizePolicy().hasHeightForWidth())
        self.threshold_label.setSizePolicy(sizePolicy)
        self.threshold_label.setMinimumSize(QtCore.QSize(0, 22))
        font = QtGui.QFont()
        font.setWeight(75)
        font.setBold(True)
        self.threshold_label.setFont(font)
        self.threshold_label.setAlignment(QtCore.Qt.AlignLeading|QtCore.Qt.AlignLeft|QtCore.Qt.AlignVCenter)
        self.threshold_label.setWordWrap(False)
        self.threshold_label.setObjectName("threshold_label")
        self.gridLayout.addWidget(self.threshold_label, 0, 2, 1, 1)
        self.temperature_label = QtGui.QLabel(self.frame)
        self.temperature_label.setEnabled(True)
        self.temperature_label.setMaximumSize(QtCore.QSize(16777215, 16777215))
        self.temperature_label.setMouseTracking(True)
        self.temperature_label.setWordWrap(True)
        self.temperature_label.setObjectName("temperature_label")
        self.gridLayout.addWidget(self.temperature_label, 2, 1, 1, 1)
        self.verticalLayout = QtGui.QVBoxLayout()
        self.verticalLayout.setSpacing(4)
        self.verticalLayout.setObjectName("verticalLayout")
        self.heater_on_btn = QtGui.QPushButton(self.frame)
        self.heater_on_btn.setIconSize(QtCore.QSize(16, 16))
        self.heater_on_btn.setObjectName("heater_on_btn")
        self.verticalLayout.addWidget(self.heater_on_btn)
        self.heater_off_btn = QtGui.QPushButton(self.frame)
        self.heater_off_btn.setObjectName("heater_off_btn")
        self.verticalLayout.addWidget(self.heater_off_btn)
        self.heater_automatic_btn = QtGui.QPushButton(self.frame)
        self.heater_automatic_btn.setObjectName("heater_automatic_btn")
        self.verticalLayout.addWidget(self.heater_automatic_btn)
        self.gridLayout.addLayout(self.verticalLayout, 2, 6, 1, 1)
        self.manual_control_label = QtGui.QLabel(self.frame)
        self.manual_control_label.setEnabled(True)
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Fixed, QtGui.QSizePolicy.Preferred)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.manual_control_label.sizePolicy().hasHeightForWidth())
        self.manual_control_label.setSizePolicy(sizePolicy)
        self.manual_control_label.setMinimumSize(QtCore.QSize(0, 22))
        font = QtGui.QFont()
        font.setWeight(75)
        font.setBold(True)
        self.manual_control_label.setFont(font)
        self.manual_control_label.setAlignment(QtCore.Qt.AlignLeading|QtCore.Qt.AlignLeft|QtCore.Qt.AlignVCenter)
        self.manual_control_label.setObjectName("manual_control_label")
        self.gridLayout.addWidget(self.manual_control_label, 0, 6, 1, 1)
        self.temperature_threshold = QtGui.QDoubleSpinBox(self.frame)
        self.temperature_threshold.setEnabled(True)
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Preferred, QtGui.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.temperature_threshold.sizePolicy().hasHeightForWidth())
        self.temperature_threshold.setSizePolicy(sizePolicy)
        self.temperature_threshold.setMinimumSize(QtCore.QSize(0, 22))
        self.temperature_threshold.setProperty("value", 25.0)
        self.temperature_threshold.setObjectName("temperature_threshold")
        self.gridLayout.addWidget(self.temperature_threshold, 2, 2, 1, 1)
        self.humidity_read_delta = QtGui.QDoubleSpinBox(self.frame)
        self.humidity_read_delta.setEnabled(True)
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Preferred, QtGui.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.humidity_read_delta.sizePolicy().hasHeightForWidth())
        self.humidity_read_delta.setSizePolicy(sizePolicy)
        self.humidity_read_delta.setMinimumSize(QtCore.QSize(0, 22))
        self.humidity_read_delta.setSingleStep(0.5)
        self.humidity_read_delta.setProperty("value", 2.0)
        self.humidity_read_delta.setObjectName("humidity_read_delta")
        self.gridLayout.addWidget(self.humidity_read_delta, 3, 4, 1, 1)
        self.humidity_label = QtGui.QLabel(self.frame)
        self.humidity_label.setEnabled(True)
        self.humidity_label.setMaximumSize(QtCore.QSize(16777215, 16777215))
        self.humidity_label.setObjectName("humidity_label")
        self.gridLayout.addWidget(self.humidity_label, 3, 1, 1, 1)
        self.verticalLayout_3 = QtGui.QVBoxLayout()
        self.verticalLayout_3.setSpacing(4)
        self.verticalLayout_3.setObjectName("verticalLayout_3")
        self.fan_on_btn = QtGui.QPushButton(self.frame)
        self.fan_on_btn.setObjectName("fan_on_btn")
        self.verticalLayout_3.addWidget(self.fan_on_btn)
        self.fan_off_btn = QtGui.QPushButton(self.frame)
        self.fan_off_btn.setObjectName("fan_off_btn")
        self.verticalLayout_3.addWidget(self.fan_off_btn)
        self.fan_automatic_btn = QtGui.QPushButton(self.frame)
        self.fan_automatic_btn.setObjectName("fan_automatic_btn")
        self.verticalLayout_3.addWidget(self.fan_automatic_btn)
        self.gridLayout.addLayout(self.verticalLayout_3, 3, 6, 1, 1)
        self.humidity_threshold = QtGui.QSpinBox(self.frame)
        self.humidity_threshold.setEnabled(True)
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Preferred, QtGui.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.humidity_threshold.sizePolicy().hasHeightForWidth())
        self.humidity_threshold.setSizePolicy(sizePolicy)
        self.humidity_threshold.setMinimumSize(QtCore.QSize(0, 22))
        self.humidity_threshold.setProperty("value", 30)
        self.humidity_threshold.setObjectName("humidity_threshold")
        self.gridLayout.addWidget(self.humidity_threshold, 3, 2, 1, 1)
        self.temperature_read_interval = QtGui.QSpinBox(self.frame)
        self.temperature_read_interval.setEnabled(True)
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Preferred, QtGui.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.temperature_read_interval.sizePolicy().hasHeightForWidth())
        self.temperature_read_interval.setSizePolicy(sizePolicy)
        self.temperature_read_interval.setMinimumSize(QtCore.QSize(0, 22))
        self.temperature_read_interval.setMinimum(100)
        self.temperature_read_interval.setMaximum(100000)
        self.temperature_read_interval.setSingleStep(100)
        self.temperature_read_interval.setProperty("value", 1000)
        self.temperature_read_interval.setObjectName("temperature_read_interval")
        self.gridLayout.addWidget(self.temperature_read_interval, 2, 5, 1, 1)
        self.temperature_orientation = QtGui.QComboBox(self.frame)
        self.temperature_orientation.setMouseTracking(True)
        self.temperature_orientation.setFocusPolicy(QtCore.Qt.WheelFocus)
        self.temperature_orientation.setEditable(False)
        self.temperature_orientation.setMaxVisibleItems(10)
        self.temperature_orientation.setInsertPolicy(QtGui.QComboBox.NoInsert)
        self.temperature_orientation.setSizeAdjustPolicy(QtGui.QComboBox.AdjustToMinimumContentsLength)
        self.temperature_orientation.setMinimumContentsLength(10)
        self.temperature_orientation.setObjectName("temperature_orientation")
        self.temperature_orientation.addItem("")
        self.temperature_orientation.addItem("")
        self.gridLayout.addWidget(self.temperature_orientation, 2, 3, 1, 1)
        self.humidity_orientation = QtGui.QComboBox(self.frame)
        self.humidity_orientation.setMouseTracking(True)
        self.humidity_orientation.setEditable(False)
        self.humidity_orientation.setMaxVisibleItems(10)
        self.humidity_orientation.setInsertPolicy(QtGui.QComboBox.NoInsert)
        self.humidity_orientation.setSizeAdjustPolicy(QtGui.QComboBox.AdjustToMinimumContentsLength)
        self.humidity_orientation.setMinimumContentsLength(10)
        self.humidity_orientation.setObjectName("humidity_orientation")
        self.humidity_orientation.addItem("")
        self.humidity_orientation.addItem("")
        self.gridLayout.addWidget(self.humidity_orientation, 3, 3, 1, 1)
        self.threshold_orientation_label = QtGui.QLabel(self.frame)
        self.threshold_orientation_label.setEnabled(True)
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Fixed, QtGui.QSizePolicy.Preferred)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.threshold_orientation_label.sizePolicy().hasHeightForWidth())
        self.threshold_orientation_label.setSizePolicy(sizePolicy)
        self.threshold_orientation_label.setMinimumSize(QtCore.QSize(0, 22))
        font = QtGui.QFont()
        font.setWeight(75)
        font.setBold(True)
        self.threshold_orientation_label.setFont(font)
        self.threshold_orientation_label.setAlignment(QtCore.Qt.AlignLeading|QtCore.Qt.AlignLeft|QtCore.Qt.AlignVCenter)
        self.threshold_orientation_label.setWordWrap(True)
        self.threshold_orientation_label.setObjectName("threshold_orientation_label")
        self.gridLayout.addWidget(self.threshold_orientation_label, 0, 3, 1, 1)
        self.relay_label = QtGui.QLabel(self.frame)
        self.relay_label.setEnabled(True)
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Fixed, QtGui.QSizePolicy.Preferred)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.relay_label.sizePolicy().hasHeightForWidth())
        self.relay_label.setSizePolicy(sizePolicy)
        self.relay_label.setMinimumSize(QtCore.QSize(0, 0))
        font = QtGui.QFont()
        font.setWeight(75)
        font.setBold(True)
        self.relay_label.setFont(font)
        self.relay_label.setScaledContents(False)
        self.relay_label.setAlignment(QtCore.Qt.AlignLeading|QtCore.Qt.AlignLeft|QtCore.Qt.AlignVCenter)
        self.relay_label.setWordWrap(False)
        self.relay_label.setObjectName("relay_label")
        self.gridLayout.addWidget(self.relay_label, 0, 0, 1, 1)
        self.heater_label = QtGui.QLabel(self.frame)
        self.heater_label.setEnabled(True)
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Preferred, QtGui.QSizePolicy.Preferred)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.heater_label.sizePolicy().hasHeightForWidth())
        self.heater_label.setSizePolicy(sizePolicy)
        self.heater_label.setMaximumSize(QtCore.QSize(16777215, 16777215))
        self.heater_label.setScaledContents(True)
        self.heater_label.setObjectName("heater_label")
        self.gridLayout.addWidget(self.heater_label, 2, 0, 1, 1)
        self.fan_label = QtGui.QLabel(self.frame)
        self.fan_label.setEnabled(True)
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Preferred, QtGui.QSizePolicy.Preferred)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.fan_label.sizePolicy().hasHeightForWidth())
        self.fan_label.setSizePolicy(sizePolicy)
        self.fan_label.setMaximumSize(QtCore.QSize(16777215, 16777215))
        self.fan_label.setScaledContents(True)
        self.fan_label.setObjectName("fan_label")
        self.gridLayout.addWidget(self.fan_label, 3, 0, 1, 1)
        self.read_interval_label = QtGui.QLabel(self.frame)
        self.read_interval_label.setEnabled(True)
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Fixed, QtGui.QSizePolicy.Preferred)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.read_interval_label.sizePolicy().hasHeightForWidth())
        self.read_interval_label.setSizePolicy(sizePolicy)
        self.read_interval_label.setMinimumSize(QtCore.QSize(0, 22))
        font = QtGui.QFont()
        font.setWeight(75)
        font.setBold(True)
        self.read_interval_label.setFont(font)
        self.read_interval_label.setAutoFillBackground(False)
        self.read_interval_label.setAlignment(QtCore.Qt.AlignLeading|QtCore.Qt.AlignLeft|QtCore.Qt.AlignTop)
        self.read_interval_label.setWordWrap(True)
        self.read_interval_label.setObjectName("read_interval_label")
        self.gridLayout.addWidget(self.read_interval_label, 0, 5, 2, 1)
        self.verticalLayout_4.addWidget(self.frame)
        spacerItem3 = QtGui.QSpacerItem(20, 16, QtGui.QSizePolicy.Minimum, QtGui.QSizePolicy.Expanding)
        self.verticalLayout_4.addItem(spacerItem3)
        self.horizontalLayout_2 = QtGui.QHBoxLayout()
        self.horizontalLayout_2.setObjectName("horizontalLayout_2")
        self.frame1 = QtGui.QFrame(self.layoutWidget)
        self.frame1.setFrameShape(QtGui.QFrame.Box)
        self.frame1.setObjectName("frame1")
        self.verticalLayout_2 = QtGui.QVBoxLayout(self.frame1)
        self.verticalLayout_2.setSpacing(11)
        self.verticalLayout_2.setObjectName("verticalLayout_2")
        self.heartbeat_label = QtGui.QLabel(self.frame1)
        self.heartbeat_label.setEnabled(True)
        self.heartbeat_label.setMinimumSize(QtCore.QSize(0, 22))
        font = QtGui.QFont()
        font.setWeight(75)
        font.setBold(True)
        self.heartbeat_label.setFont(font)
        self.heartbeat_label.setAlignment(QtCore.Qt.AlignLeading|QtCore.Qt.AlignLeft|QtCore.Qt.AlignVCenter)
        self.heartbeat_label.setObjectName("heartbeat_label")
        self.verticalLayout_2.addWidget(self.heartbeat_label)
        self.gridLayout_2 = QtGui.QGridLayout()
        self.gridLayout_2.setHorizontalSpacing(31)
        self.gridLayout_2.setVerticalSpacing(4)
        self.gridLayout_2.setObjectName("gridLayout_2")
        self.sensor_heartbeat_label = QtGui.QLabel(self.frame1)
        self.sensor_heartbeat_label.setEnabled(True)
        self.sensor_heartbeat_label.setMinimumSize(QtCore.QSize(0, 22))
        font = QtGui.QFont()
        font.setWeight(50)
        font.setBold(False)
        self.sensor_heartbeat_label.setFont(font)
        self.sensor_heartbeat_label.setAlignment(QtCore.Qt.AlignLeading|QtCore.Qt.AlignLeft|QtCore.Qt.AlignVCenter)
        self.sensor_heartbeat_label.setObjectName("sensor_heartbeat_label")
        self.gridLayout_2.addWidget(self.sensor_heartbeat_label, 0, 0, 1, 1)
        self.sensor_heartbeat = QtGui.QSpinBox(self.frame1)
        self.sensor_heartbeat.setEnabled(True)
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Preferred, QtGui.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.sensor_heartbeat.sizePolicy().hasHeightForWidth())
        self.sensor_heartbeat.setSizePolicy(sizePolicy)
        self.sensor_heartbeat.setMinimumSize(QtCore.QSize(0, 22))
        self.sensor_heartbeat.setMinimum(1)
        self.sensor_heartbeat.setMaximum(100000)
        self.sensor_heartbeat.setSingleStep(1)
        self.sensor_heartbeat.setProperty("value", 15)
        self.sensor_heartbeat.setObjectName("sensor_heartbeat")
        self.gridLayout_2.addWidget(self.sensor_heartbeat, 0, 1, 1, 1)
        self.actuator_heartbeat_label = QtGui.QLabel(self.frame1)
        self.actuator_heartbeat_label.setEnabled(True)
        self.actuator_heartbeat_label.setMinimumSize(QtCore.QSize(0, 22))
        font = QtGui.QFont()
        font.setWeight(50)
        font.setBold(False)
        self.actuator_heartbeat_label.setFont(font)
        self.actuator_heartbeat_label.setAlignment(QtCore.Qt.AlignLeading|QtCore.Qt.AlignLeft|QtCore.Qt.AlignVCenter)
        self.actuator_heartbeat_label.setObjectName("actuator_heartbeat_label")
        self.gridLayout_2.addWidget(self.actuator_heartbeat_label, 1, 0, 1, 1)
        self.actuator_heartbeat = QtGui.QSpinBox(self.frame1)
        self.actuator_heartbeat.setEnabled(True)
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Preferred, QtGui.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.actuator_heartbeat.sizePolicy().hasHeightForWidth())
        self.actuator_heartbeat.setSizePolicy(sizePolicy)
        self.actuator_heartbeat.setMinimumSize(QtCore.QSize(0, 22))
        self.actuator_heartbeat.setMaximum(100000)
        self.actuator_heartbeat.setSingleStep(1)
        self.actuator_heartbeat.setProperty("value", 15)
        self.actuator_heartbeat.setObjectName("actuator_heartbeat")
        self.gridLayout_2.addWidget(self.actuator_heartbeat, 1, 1, 1, 1)
        self.controller_heartbeat_label = QtGui.QLabel(self.frame1)
        self.controller_heartbeat_label.setEnabled(True)
        self.controller_heartbeat_label.setMinimumSize(QtCore.QSize(0, 22))
        font = QtGui.QFont()
        font.setWeight(50)
        font.setBold(False)
        self.controller_heartbeat_label.setFont(font)
        self.controller_heartbeat_label.setAlignment(QtCore.Qt.AlignLeading|QtCore.Qt.AlignLeft|QtCore.Qt.AlignVCenter)
        self.controller_heartbeat_label.setObjectName("controller_heartbeat_label")
        self.gridLayout_2.addWidget(self.controller_heartbeat_label, 2, 0, 1, 1)
        self.controller_heartbeat = QtGui.QSpinBox(self.frame1)
        self.controller_heartbeat.setEnabled(True)
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Preferred, QtGui.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.controller_heartbeat.sizePolicy().hasHeightForWidth())
        self.controller_heartbeat.setSizePolicy(sizePolicy)
        self.controller_heartbeat.setMinimumSize(QtCore.QSize(0, 22))
        self.controller_heartbeat.setMaximum(100000)
        self.controller_heartbeat.setSingleStep(1)
        self.controller_heartbeat.setProperty("value", 15)
        self.controller_heartbeat.setObjectName("controller_heartbeat")
        self.gridLayout_2.addWidget(self.controller_heartbeat, 2, 1, 1, 1)
        self.verticalLayout_2.addLayout(self.gridLayout_2)
        self.horizontalLayout_2.addWidget(self.frame1)
        spacerItem4 = QtGui.QSpacerItem(40, 20, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum)
        self.horizontalLayout_2.addItem(spacerItem4)
        self.verticalLayout_4.addLayout(self.horizontalLayout_2)
        spacerItem5 = QtGui.QSpacerItem(20, 36, QtGui.QSizePolicy.Minimum, QtGui.QSizePolicy.Expanding)
        self.verticalLayout_4.addItem(spacerItem5)
        self.horizontalLayout = QtGui.QHBoxLayout()
        self.horizontalLayout.setObjectName("horizontalLayout")
        spacerItem6 = QtGui.QSpacerItem(40, 20, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum)
        self.horizontalLayout.addItem(spacerItem6)
        self.save_settings_btn = QtGui.QPushButton(self.layoutWidget)
        self.save_settings_btn.setEnabled(True)
        self.save_settings_btn.setMaximumSize(QtCore.QSize(100, 16777215))
        self.save_settings_btn.setObjectName("save_settings_btn")
        self.horizontalLayout.addWidget(self.save_settings_btn)
        spacerItem7 = QtGui.QSpacerItem(40, 20, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum)
        self.horizontalLayout.addItem(spacerItem7)
        self.verticalLayout_4.addLayout(self.horizontalLayout)
        AdminMainWindow.setCentralWidget(self.centralwidget)
        self.statusbar = QtGui.QStatusBar(AdminMainWindow)
        self.statusbar.setObjectName("statusbar")
        AdminMainWindow.setStatusBar(self.statusbar)

        self.retranslateUi(AdminMainWindow)
        self.temperature_orientation.setCurrentIndex(1)
        self.humidity_orientation.setCurrentIndex(0)
        QtCore.QMetaObject.connectSlotsByName(AdminMainWindow)

    def retranslateUi(self, AdminMainWindow):
        AdminMainWindow.setWindowTitle(QtGui.QApplication.translate("AdminMainWindow", "Flow Climate Control Admin", None, QtGui.QApplication.UnicodeUTF8))
        self.connection_status_label.setText(QtGui.QApplication.translate("AdminMainWindow", "Connection status:", None, QtGui.QApplication.UnicodeUTF8))
        self.connection_status.setText(QtGui.QApplication.translate("AdminMainWindow", "OK", None, QtGui.QApplication.UnicodeUTF8))
        self.sensor_label.setText(QtGui.QApplication.translate("AdminMainWindow", "Sensor", None, QtGui.QApplication.UnicodeUTF8))
        self.read_delta_label.setText(QtGui.QApplication.translate("AdminMainWindow", "Read delta", None, QtGui.QApplication.UnicodeUTF8))
        self.threshold_label.setText(QtGui.QApplication.translate("AdminMainWindow", "Threshold", None, QtGui.QApplication.UnicodeUTF8))
        self.temperature_label.setText(QtGui.QApplication.translate("AdminMainWindow", "Temperature", None, QtGui.QApplication.UnicodeUTF8))
        self.heater_on_btn.setText(QtGui.QApplication.translate("AdminMainWindow", "Force On", None, QtGui.QApplication.UnicodeUTF8))
        self.heater_off_btn.setText(QtGui.QApplication.translate("AdminMainWindow", "Force Off", None, QtGui.QApplication.UnicodeUTF8))
        self.heater_automatic_btn.setText(QtGui.QApplication.translate("AdminMainWindow", "Automatic", None, QtGui.QApplication.UnicodeUTF8))
        self.manual_control_label.setText(QtGui.QApplication.translate("AdminMainWindow", "Manual control", None, QtGui.QApplication.UnicodeUTF8))
        self.humidity_label.setText(QtGui.QApplication.translate("AdminMainWindow", "Humidity(%RH)", None, QtGui.QApplication.UnicodeUTF8))
        self.fan_on_btn.setText(QtGui.QApplication.translate("AdminMainWindow", "Force On", None, QtGui.QApplication.UnicodeUTF8))
        self.fan_off_btn.setText(QtGui.QApplication.translate("AdminMainWindow", "Force Off", None, QtGui.QApplication.UnicodeUTF8))
        self.fan_automatic_btn.setText(QtGui.QApplication.translate("AdminMainWindow", "Automatic", None, QtGui.QApplication.UnicodeUTF8))
        self.temperature_orientation.setItemText(0, QtGui.QApplication.translate("AdminMainWindow", "On if above", None, QtGui.QApplication.UnicodeUTF8))
        self.temperature_orientation.setItemText(1, QtGui.QApplication.translate("AdminMainWindow", "On if below", None, QtGui.QApplication.UnicodeUTF8))
        self.humidity_orientation.setItemText(0, QtGui.QApplication.translate("AdminMainWindow", "On if above", None, QtGui.QApplication.UnicodeUTF8))
        self.humidity_orientation.setItemText(1, QtGui.QApplication.translate("AdminMainWindow", "On if below", None, QtGui.QApplication.UnicodeUTF8))
        self.threshold_orientation_label.setText(QtGui.QApplication.translate("AdminMainWindow", "Threshold orientation", None, QtGui.QApplication.UnicodeUTF8))
        self.relay_label.setText(QtGui.QApplication.translate("AdminMainWindow", "Relay", None, QtGui.QApplication.UnicodeUTF8))
        self.heater_label.setText(QtGui.QApplication.translate("AdminMainWindow", "Heater", None, QtGui.QApplication.UnicodeUTF8))
        self.fan_label.setText(QtGui.QApplication.translate("AdminMainWindow", "Fan", None, QtGui.QApplication.UnicodeUTF8))
        self.read_interval_label.setText(QtGui.QApplication.translate("AdminMainWindow", "Read interval (msec)", None, QtGui.QApplication.UnicodeUTF8))
        self.heartbeat_label.setText(QtGui.QApplication.translate("AdminMainWindow", "Heartbeat(sec)", None, QtGui.QApplication.UnicodeUTF8))
        self.sensor_heartbeat_label.setText(QtGui.QApplication.translate("AdminMainWindow", "Sensor", None, QtGui.QApplication.UnicodeUTF8))
        self.actuator_heartbeat_label.setText(QtGui.QApplication.translate("AdminMainWindow", "Actuator", None, QtGui.QApplication.UnicodeUTF8))
        self.controller_heartbeat_label.setText(QtGui.QApplication.translate("AdminMainWindow", "Controller", None, QtGui.QApplication.UnicodeUTF8))
        self.save_settings_btn.setText(QtGui.QApplication.translate("AdminMainWindow", "Save", None, QtGui.QApplication.UnicodeUTF8))

import resource_rc