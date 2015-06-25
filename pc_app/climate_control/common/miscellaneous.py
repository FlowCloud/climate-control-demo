""" This module contains code common to Admin app and Display app

"""

import os
import time
import logging
import argparse


LOGGER = logging.getLogger(__name__)


def configure_commandline_args():
    """ Configures command line options, supports parsing log_level and showing version

    :return args object, currently supports log_level parameter
    """
    parser = argparse.ArgumentParser(description='Climate control app')
    parser.add_argument("--log-level", type=str,
                        choices=[logging.getLevelName(logging.DEBUG),
                                 logging.getLevelName(logging.INFO),
                                 logging.getLevelName(logging.WARNING),
                                 logging.getLevelName(logging.ERROR),
                                 logging.getLevelName(logging.CRITICAL)],
                        help='log level for the application, log will be added to logs folder')
    internal_version, external_version = get_app_version()
    parser.add_argument("--version", action="version",
                        version="External version={} Internal version={}".
                        format(external_version, internal_version))
    args = parser.parse_args()
    return args


def configure_logs(app_name):
    """ Configures the log format, starts with log level info

    :param str app_name: app name to be prefixed to the log level e.g "display", "admin"
    """
    # start with log level INFO
    numeric_level = logging.INFO

    logs_dir = os.path.join(os.path.dirname(__file__), os.pardir, "logs")
    if not os.path.isdir(logs_dir):
        os.makedirs(logs_dir)

    file_name = "{}_log_{}.txt".format(app_name, time.strftime("%d-%m-%Y_%H-%M-%S"))
    logging.basicConfig(filename=os.path.join(os.path.dirname(__file__), os.pardir, "logs",
                                              file_name),
                        format="%(asctime)s %(levelname)-8s %(name)-18s %(funcName)-30s "
                               "- %(message)s",
                        datefmt="[%d-%m-%Y %H:%M:%S]", level=numeric_level)


def set_log_level(log_level):
    """ Sets the log level

    :param str log_level: one of these log levels "DEBUG", "INFO", "WARNING", "ERROR", "CRITICAL"
    """
    if log_level:
        numeric_level = getattr(logging, log_level.upper(), None)
        root_logger = logging.getLogger()
        root_logger.setLevel(numeric_level)


def get_app_version():
    """ Parses and returns the internal and external app version from version.txt

    :return str internal_version: internal version string
    :return str external_version: external version string
    """
    internal_version = None
    external_version = None
    try:
        with open(os.path.join(os.path.dirname(__file__), os.pardir, "version.txt"), "r") \
                as version_file:
            pairs = (line.strip().split("=") for line in version_file)
            data = [(name, value) for name, value in pairs]
            values = dict(data)
            internal_version = values["INTERNAL_VERSION"]
            external_version = values["EXTERNAL_VERSION"]
    except (IOError, ValueError, KeyError) as error:
        LOGGER.exception(error)
    return internal_version, external_version
