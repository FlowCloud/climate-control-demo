""" Tests for ControllerSetting class
"""


import sys
import os
sys.path.append(os.path.join(os.path.dirname(__file__), os.path.pardir))
import unittest
from common.model import ControllerSetting


class TestControllerSetting(unittest.TestCase):
    """ Test ControllerSetting class
    """

    def test_controller_setting_create_from_dictionary_success(self):
        """ Test passes when setting object is created and created xml is valid

        """
        setting = {"temperature_threshold": 25.0,
                   "humidity_threshold": 35.0,
                   "temperature_orientation": "ABOVE",
                   "humidity_orientation": "BELOW",
                   "temperature_read_interval": 1000,
                   "humidity_read_interval": 2500,
                   "temperature_read_delta": 0.5,
                   "humidity_read_delta": 2.0,
                   "sensor_heartbeat": 15,
                   "actuator_heartbeat": 15,
                   "controller_heartbeat": 15}
        xml = "<ControllerConfig>"\
              "<version>1.0</version>"\
              "<TemperatureThreshold>25.00</TemperatureThreshold>"\
              "<HumidityThreshold>35.00</HumidityThreshold>"\
              "<TemperatureOrientation>ABOVE</TemperatureOrientation>"\
              "<HumidityOrientation>BELOW</HumidityOrientation>"\
              "<HeartBeat>15000</HeartBeat>"\
              "<SensorConfig>"\
              "<HeartBeat>15000</HeartBeat>"\
              "<TemperatureReadInterval>1000</TemperatureReadInterval>"\
              "<HumidityReadInterval>2500</HumidityReadInterval>"\
              "<TemperatureReadDelta>0.5</TemperatureReadDelta>"\
              "<HumidityReadDelta>2.0</HumidityReadDelta>"\
              "</SensorConfig>"\
              "<ActuatorConfig>"\
              "<HeartBeat>15000</HeartBeat>"\
              "</ActuatorConfig>"\
              "</ControllerConfig>"

        controller_setting = ControllerSetting(setting=setting)
        self.assertIsNotNone(controller_setting)
        self.assertEqual(controller_setting.to_xml(), xml)

    def test_controller_setting_create_from_xml_success(self):
        """ Test passes when setting is successfully created from xml and converted xml is
        same as before

        """
        xml = "<ControllerConfig>"\
              "<version>1.0</version>"\
              "<TemperatureThreshold>25.00</TemperatureThreshold>"\
              "<HumidityThreshold>35.00</HumidityThreshold>"\
              "<TemperatureOrientation>ABOVE</TemperatureOrientation>"\
              "<HumidityOrientation>BELOW</HumidityOrientation>"\
              "<HeartBeat>15000</HeartBeat>"\
              "<SensorConfig>"\
              "<HeartBeat>15000</HeartBeat>"\
              "<TemperatureReadInterval>1000</TemperatureReadInterval>"\
              "<HumidityReadInterval>2500</HumidityReadInterval>"\
              "<TemperatureReadDelta>0.5</TemperatureReadDelta>"\
              "<HumidityReadDelta>2.0</HumidityReadDelta>"\
              "</SensorConfig>"\
              "<ActuatorConfig>"\
              "<HeartBeat>15000</HeartBeat>"\
              "</ActuatorConfig>"\
              "</ControllerConfig>"
        controller_setting = ControllerSetting(setting_xml=xml)
        self.assertIsNotNone(controller_setting)
        self.assertEqual(controller_setting.temperature_threshold, 25.0)
        self.assertEqual(controller_setting.humidity_threshold, 35.0)
        self.assertEqual(controller_setting.temperature_orientation, "ABOVE")
        self.assertEqual(controller_setting.humidity_orientation, "BELOW")
        self.assertEqual(controller_setting.temperature_read_delta, 0.5)
        self.assertEqual(controller_setting.humidity_read_delta, 2.0)
        self.assertEqual(controller_setting.temperature_read_interval, 1000)
        self.assertEqual(controller_setting.humidity_read_interval, 2500)
        # heartbeat is converted to sec from msec, so check that
        self.assertEqual(controller_setting.controller_heartbeat, 15)
        self.assertEqual(controller_setting.sensor_heartbeat, 15)
        self.assertEqual(controller_setting.actuator_heartbeat, 15)
        self.assertEqual(controller_setting.to_xml(), xml)

    def test_controller_setting_invalid_xml_raises_value_error(self):
        """ Test ValueError is raised if invalid xml is passed

        """
        xml = "<ControllerConfig>"
        with self.assertRaises(ValueError):
            ControllerSetting(setting_xml=xml)

    def test_controller_setting_missing_tag_in_xml_raises_value_error(self):
        """ Test ValueError is raised if some xml tags are missing

        """
        # most of the required xml tags are missing
        xml = "<ControllerConfig>"\
              "<version>1.0</version>"\
              "<TemperatureThreshold>25.00</TemperatureThreshold>"\
              "</ControllerConfig>"
        with self.assertRaises(ValueError):
            ControllerSetting(setting_xml=xml)

    def test_controller_setting_invalid_temperature_orientation_raises_value_error(self):
        """ Test ValueError is raised if TemperatureOrientation is not ABOVE or BELOW

        """
        xml = "<ControllerConfig>"\
              "<version>1.0</version>"\
              "<TemperatureThreshold>25.00</TemperatureThreshold>"\
              "<HumidityThreshold>35.00</HumidityThreshold>"\
              "<TemperatureOrientation>Above</TemperatureOrientation>"\
              "<HumidityOrientation>BELOW</HumidityOrientation>"\
              "<HeartBeat>15000</HeartBeat>"\
              "<SensorConfig>"\
              "<HeartBeat>15000</HeartBeat>"\
              "<TemperatureReadInterval>1000</TemperatureReadInterval>"\
              "<HumidityReadInterval>2500</HumidityReadInterval>"\
              "<TemperatureReadDelta>0.5</TemperatureReadDelta>"\
              "<HumidityReadDelta>2.0</HumidityReadDelta>"\
              "</SensorConfig>"\
              "<ActuatorConfig>"\
              "<HeartBeat>15000</HeartBeat>"\
              "</ActuatorConfig>"\
              "</ControllerConfig>"
        with self.assertRaises(ValueError):
            ControllerSetting(setting_xml=xml)

    def test_controller_setting_invalid_humidity_orientation_raises_value_error(self):
        """ Test ValueError is raised if HumidityOrientation is not ABOVE or BELOW

        """
        xml = "<ControllerConfig>"\
              "<version>1.0</version>"\
              "<TemperatureThreshold>25.00</TemperatureThreshold>"\
              "<HumidityThreshold>35.00</HumidityThreshold>"\
              "<TemperatureOrientation>ABOVE</TemperatureOrientation>"\
              "<HumidityOrientation>Below</HumidityOrientation>"\
              "<HeartBeat>15000</HeartBeat>"\
              "<SensorConfig>"\
              "<HeartBeat>15000</HeartBeat>"\
              "<TemperatureReadInterval>1000</TemperatureReadInterval>"\
              "<HumidityReadInterval>2500</HumidityReadInterval>"\
              "<TemperatureReadDelta>0.5</TemperatureReadDelta>"\
              "<HumidityReadDelta>2.0</HumidityReadDelta>"\
              "</SensorConfig>"\
              "<ActuatorConfig>"\
              "<HeartBeat>15000</HeartBeat>"\
              "</ActuatorConfig>"\
              "</ControllerConfig>"

        with self.assertRaises(ValueError):
            ControllerSetting(setting_xml=xml)


if __name__ == '__main__':
    unittest.main()
