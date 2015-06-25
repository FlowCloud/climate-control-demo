""" Tests for FlowDevice class
"""
import sys
import os
sys.path.append(os.path.join(os.path.dirname(__file__), *([os.path.pardir] * 1)))
import unittest
from libflow.flow import FlowUser, flow_initialize, FlowCoreException
SETTING_KEY = "project_name"
SETTING_VALUE = "climate_control"


class TestFlowDevice(unittest.TestCase):
    """ Testing of FlowDevice class members
    """
    def setUp(self):
        flow_initialize(flow_server_url, flow_server_key, flow_server_secret)
        self.user = FlowUser(username, password)
        self.device = self.user.find_device(device_type)

    def test_get_device_type_expected(self):
        """ Test to check get_device_type()
        """
        self.assertEqual(self.device.get_device_type(), device_type)

    def test_get_device_name_expected(self):
        """ Test to check get_device_name()
        """
        self.assertIsNotNone(self.device.get_device_name())

    def test_get_device_id_expected(self):
        """ Test to check get_device_id()
        """
        self.assertIsNotNone(self.device.get_device_id())

    def test_save_setting(self):
        """ Test to check save_settings()
        """
        self.device.save_setting(SETTING_KEY, SETTING_VALUE)

    def test_save_setting_none_parameters_raises_value_error(self):
        """ Test passes when save_settings() raises ValueError
        """
        with self.assertRaises(ValueError):
            self.device.save_setting("", "")

    def test_get_setting_expected(self):
        """ Test to check get_settings()
        """
        self.device.save_setting(SETTING_KEY, SETTING_VALUE)
        self.assertEqual(self.device.get_setting(SETTING_KEY), SETTING_VALUE)

    def test_get_setting_wrong_parameter_raises_flow_exception(self):
        """ Test passes when get_settings() raises FlowCoreException
        """
        with self.assertRaises(FlowCoreException):
            self.device.get_setting("unknown_key")

    def test_get_setting_none_parameter_raises_value_error(self):
        """ Test passes when get_setting() raises ValueError
        """
        with self.assertRaises(ValueError):
            self.device.get_setting("")

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
