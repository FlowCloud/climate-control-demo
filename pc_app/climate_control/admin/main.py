""" Entry point of admin application
Also initializes logging to a log file based on the command line arguments
"""

import sys
import os
import logging
from PySide.QtGui import QApplication, QStyleFactory
from .controller import AdminController
from common.ui.stylesheet import STYLESHEET
from common.miscellaneous import configure_commandline_args, configure_logs, set_log_level
from libflow.flow import flow_set_nvs_file


LOGGER = logging.getLogger(__name__)


def main():
    """ Entry point of application

    """
    configure_logs("admin")
    args = configure_commandline_args()
    set_log_level(args.log_level)

    LOGGER.info("Application started")
    app = QApplication(sys.argv)
    # App window style options can be get by printing QStyleFactory.keys()
    QApplication.setStyle(QStyleFactory.create("GTK+"))  # Set style of Heater App to GTK
    app.setStyleSheet(STYLESHEET)
    flow_nvs_file = os.path.join(os.path.dirname(__file__), os.pardir, "libflow",
                                 "admin-libflow-nvs")
    flow_set_nvs_file(flow_nvs_file)
    dummy_controller = AdminController()
    sys.exit(app.exec_())
