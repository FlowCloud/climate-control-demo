""" This module contains API's to check internet and network status
"""
import logging
import netifaces
import subprocess

LOGGER = logging.getLogger(__name__)
DNS_SERVER = "8.8.8.8"


class NetworkMonitor(object):
    """ Class to check network and internet status
    """

    def get_connection_status(self):
        """ Get network and internet status

        :return: dictionary containing connection status
        :rtype: dict
        """
        status = {"network": True, "internet": True}
        if not self.__is_network_up():
            status["network"] = False
            status["internet"] = False
        elif not self.__is_internet_up():
            status["internet"] = False
        return status

    def __is_network_up(self):
        """ Is network connected or disconnected

        :return: True If any of the interface i.e eth0/wlan0 is up
        :rtype: bool
        """
        for interface in netifaces.interfaces():
            if self.__is_interface_up(interface):
                return True
        return False

    @staticmethod
    def __is_internet_up():
        """ Check if Internet is accessible or not
        by pinging the dns server.

        :return: True If Internet is accessible
        :rtype: bool
        """
        try:
            subprocess.check_output("ping -c 2 -W 2 {}".format(DNS_SERVER),
                                    stderr=subprocess.STDOUT, shell="False")
            return True
        except subprocess.CalledProcessError as error:
            LOGGER.exception("ping to dns server failed {}, \
            error msg: {}".format(DNS_SERVER, error.message))

        return False

    @staticmethod
    def __is_interface_up(interface):
        """ Is any one of the interface up or not

        :param str interface: interface to check, i.e eth0/wlan0 etc
        :return: True if ip address is assigned to the interface
        :rtype: bool
        """
        address = netifaces.ifaddresses(interface)
        # ignore the loopback interface as it will always have 127.0.0.1 assigned to it
        return netifaces.AF_INET in address and address[netifaces.AF_INET][0]["addr"] != "127.0.0.1"
