""" Stylesheet for admin and display app

    Same style sheet applies to admin and display app so if some widget names are same then it may
    cause undesired result
"""

STYLESHEET = """
                QWidget { background: white; font: 15pt}

                /* purple */
                QTabBar::tab:selected {background: rgb(122, 19, 117); color: white}

                /* default Labels to purple */
                QLabel {color: rgb(122, 19, 117)}

                /* login screen */
                QLabel#login_label {font: bold}
                QLabel#login_status {color: red }
                QLabel#login_connection_status {color: red }

                /* admin */
                QLabel#connection_status_label,
                QLabel#relay_label,
                QLabel#sensor_label,
                QLabel#threshold_label,
                QLabel#threshold_orientation_label,
                QLabel#read_delta_label,
                QLabel#read_interval_label,
                QLabel#manual_control_label,
                QLabel#connection_status_label,
                QLabel#heartbeat_label {font: bold}
                QComboBox{selection-color: black;}

                /* display */
                QLabel#connection_status_label,
                QLabel#latency_label,
                QLabel#controller_label,
                QLabel#sensor_label,
                QLabel#actuator_label,
                QLabel#heater_status_label,
                QLabel#fan_status_label {font: bold}

                /* Button text white and background green*/
                QPushButton { color: white; background-color: rgb(42, 126, 70) }
                QToolTip {font: 12pt}
                QStatusBar{color: red; font: 13pt}
            """
