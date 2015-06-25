""" Tests for FlowUser class
"""
import sys
import os
sys.path.append(os.path.join(os.path.dirname(__file__), *([os.path.pardir] * 1)))
import unittest
from libflow.flow import FlowUser, flow_initialize, FlowCoreException, FlowException


class TestFlowUser(unittest.TestCase):
    """ Testing of FlowUser class members
    """
    def setUp(self):
        flow_initialize(flow_server_url, flow_server_key, flow_server_secret)
        self.user = None

    def test_login(self):
        """ Test passes when user login succeed
        """
        self.user = FlowUser(username, password)

    def test_login_none_parameter_raises_value_exception(self):
        """ Test passes when user login raises ValueError
        """
        with self.assertRaises(ValueError):
            self.user = FlowUser(username, "")

    def test_login_wrong_parameters_raises_flow_exception(self):
        """ Test passes when user login raises FlowCoreException
        """
        with self.assertRaises(FlowCoreException):
            self.user = FlowUser("unknown_user", "123")

    def test_logout(self):
        """ Test to check logout()
        """
        self.user = FlowUser(username, password)
        self.user.logout()

    def test_enable_message_reception(self):
        """ Test to check enable_message_reception()
        """
        self.user = FlowUser(username, password)
        self.user.enable_message_reception("callback")

    def test_enable_message_reception_none_parameter__raises_value_error(self):
        """ Test passes when enable_message_reception() raises ValueError
        """
        self.user = FlowUser(username, password)
        with self.assertRaises(ValueError):
            self.user.enable_message_reception("")

    def test_find_device_expected(self):
        """ Test to check find_device()
        """
        self.user = FlowUser(username, password)
        self.assertIsNotNone(self.user.find_device(device_type))

    def test_find_device_none_parameter_raises_value_error(self):
        """ Test passes when find_device() raises ValueError
        """
        self.user = FlowUser(username, password)
        with self.assertRaises(ValueError):
            self.user.find_device("")

    def test_find_device_wrong_parameter_raises_flow_exception(self):
        """Test passes when no device of particular type found
        """
        self.user = FlowUser(username, password)
        with self.assertRaises(FlowException):
            self.user.find_device("unknown_device")

    def test_get_owned_devices_expected(self):
        """ Test passes when at least one device found
        """
        self.user = FlowUser(username, password)
        self.assertIsNotNone(self.user.get_owned_devices())

    def test_send_message(self):
        """ Test for sending message to device
        """
        self.user = FlowUser(username, password)
        device = self.user.find_device(device_type)
        self.user.send_message_to_device(device, "Hi", 20)

    def test_send_message_none_parameter_raises_value_error(self):
        """ Test passes when send_message_to_device() raises ValueError
        """
        self.user = FlowUser(username, password)
        device = self.user.find_device(device_type)
        with self.assertRaises(ValueError):
            self.user.send_message_to_device(device, "", 20)

if __name__ == '__main__':
    try:
        username = os.environ["flow_username"]
        password = os.environ["flow_password"]
        device_type = os.environ["flow_device_type"]
        flow_server_url = os.environ["flow_server_url"]
        flow_server_key = os.environ["flow_server_key"]
        flow_server_secret = os.environ["flow_server_secret"]
        unittest.main()
    except KeyError:
        print "Please provide your FlowCloud user credentials, device type of any registered " \
              "device and flow server credentials by setting environment variables -" \
              "flow_username, flow_password, flow_device_type, flow_server_url, " \
              "flow_server_key and flow_server_secret"
