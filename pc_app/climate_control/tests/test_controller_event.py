""" Test for ControllerEventFactory class
"""
import sys
import os
import xmltodict
sys.path.append(os.path.join(os.path.dirname(__file__), *([os.path.pardir] * 1)))
import unittest
from common.message_parse import ControllerEventFactory, MeasurementEvent, HeartBeatEvent,\
    RelayStatusEvent, DeviceStatusEvent


class TestControllerEventFactory(unittest.TestCase):
    """ Testing of ControllerEventFactory class
    """

    def test_controller_event_factory_with_valid_dict_expected(self):
        """ Test passes when valid dict is passed to ControllerEventFactory and appropriate event
        object is received

        """
        heartbeat_xml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" \
                        "<event>" \
                        "<time type=\"datetime\">2014-11-28T15:30:09Z</time>" \
                        "<type>HeartBeat</type>" \
                        "<info>"\
                        "<Temperature>25.00</Temperature>"\
                        "<Humidity>30.00</Humidity>"\
                        "<Relay_1>"\
                        "<mode>AUTO</mode>"\
                        "<status>ON</status>"\
                        "</Relay_1>"\
                        "<Relay_2>"\
                        "<mode>AUTO</mode>"\
                        "<status>ON</status>"\
                        "</Relay_2>"\
                        "<Sensor>DEAD</Sensor>"\
                        "<Actuator>DEAD</Actuator>"\
                        "</info>"\
                        "</event>"

        relay_status_xml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"\
                           "<event>"\
                           "<time type=\"datetime\">2014-11-28T15:30:09Z</time>"\
                           "<type>RelayStatus</type>"\
                           "<info>"\
                           "<Relay_1>"\
                           "<mode>AUTO</mode>"\
                           "<status>ON</status>"\
                           "</Relay_1>"\
                           "<Relay_2>"\
                           "<mode>AUTO</mode>"\
                           "<status>ON</status>"\
                           "</Relay_2>"\
                           "</info>" \
                           "</event>"

        measurement_xml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"\
                          "<event>"\
                          "<time type=\"datetime\">2014-11-28T15:30:09Z</time>"\
                          "<type>Measurement</type>"\
                          "<info>"\
                          "<Temperature>25.00</Temperature>"\
                          "<Humidity>30.00</Humidity>"\
                          "</info>"\
                          "</event>"

        device_status_xml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"\
                            "<event>"\
                            "<time type=\"datetime\">2014-11-28T15:30:09Z</time>"\
                            "<type>DeviceStatus</type>"\
                            "<info>"\
                            "<Sensor>DEAD</Sensor>"\
                            "<Actuator>DEAD</Actuator>"\
                            "</info>"\
                            "</event>"

        heartbeat_event_dict = xmltodict.parse(heartbeat_xml)
        measurement_event_dict = xmltodict.parse(measurement_xml)
        relay_status_event_dict = xmltodict.parse(relay_status_xml)
        device_status_event_dict = xmltodict.parse(device_status_xml)

        heartbeat_event = ControllerEventFactory.create_event(heartbeat_event_dict)
        measurement_event = ControllerEventFactory.create_event(measurement_event_dict)
        relay_status_event = ControllerEventFactory.create_event(relay_status_event_dict)
        device_status_event = ControllerEventFactory.create_event(device_status_event_dict)

        self.assertIsInstance(heartbeat_event, HeartBeatEvent)
        self.assertIsInstance(measurement_event, MeasurementEvent)
        self.assertIsInstance(relay_status_event, RelayStatusEvent)
        self.assertIsInstance(device_status_event, DeviceStatusEvent)

        self.assertEqual(heartbeat_event.measurement_data, measurement_event)
        self.assertEqual(heartbeat_event.relay_status, relay_status_event)
        self.assertEqual(heartbeat_event.device_status, device_status_event)
        self.assertEqual(measurement_event.temperature, 25)
        self.assertEqual(measurement_event.humidity, 30)
        self.assertEqual(device_status_event.sensor_alive, False)
        self.assertEqual(device_status_event.actuator_alive, False)
        self.assertEqual(relay_status_event.relay_1_on, True)
        self.assertEqual(relay_status_event.relay_2_on, True)
        self.assertEqual(relay_status_event.relay_1_mode, "AUTO")
        self.assertEqual(relay_status_event.relay_2_mode, "AUTO")

    def test_controller_event_with_invalid_dict_raises_value_error(self):
        """  Test if ValueError is raised when invalid dict is passed to ControllerEventFactory

        """
        # temperature tag is missing in heartbeat_xml
        heartbeat_xml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" \
                        "<event>" \
                        "<time type=\"datetime\">2014-11-28T15:30:09Z</time>" \
                        "<type>HeartBeat</type>" \
                        "<info>"\
                        "<Humidity>30.00</Humidity>"\
                        "<Relay_1>"\
                        "<mode>AUTO</mode>"\
                        "<status>ON</status>"\
                        "</Relay_1>"\
                        "<Relay_2>"\
                        "<mode>AUTO</mode>"\
                        "<status>ON</status>"\
                        "</Relay_2>"\
                        "<Sensor>DEAD</Sensor>"\
                        "<Actuator>DEAD</Actuator>"\
                        "</info>"\
                        "</event>"

        # relay_1 tag is missing in relay_status_xml
        relay_status_xml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"\
                           "<event>"\
                           "<time type=\"datetime\">2014-11-28T15:30:09Z</time>"\
                           "<type>RelayStatus</type>"\
                           "<info>"\
                           "<relay_2>"\
                           "<mode>AUTO</mode>"\
                           "<status>ON</status>"\
                           "</relay_2>"\
                           "</info>" \
                           "</event>"

        # temperature tag is missing in measurement_xml
        measurement_xml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"\
                          "<event>"\
                          "<time type=\"datetime\">2014-11-28T15:30:09Z</time>"\
                          "<type>Measurement</type>"\
                          "<info>"\
                          "<humidity>30.00</humidity>"\
                          "</info>"\
                          "</event>"

        # sensor tag is missing in device_status_xml
        device_status_xml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"\
                            "<event>"\
                            "<time type=\"datetime\">2014-11-28T15:30:09Z</time>"\
                            "<type>DeviceStatus</type>"\
                            "<info>"\
                            "<actuator>ALIVE</actuator>"\
                            "</info>"\
                            "</event>"

        heartbeat_event_dict = xmltodict.parse(heartbeat_xml)
        measurement_event_dict = xmltodict.parse(measurement_xml)
        relay_status_event_dict = xmltodict.parse(relay_status_xml)
        device_status_event_dict = xmltodict.parse(device_status_xml)

        with self.assertRaises(ValueError):
            ControllerEventFactory.create_event(heartbeat_event_dict)
        with self.assertRaises(ValueError):
            ControllerEventFactory.create_event(measurement_event_dict)
        with self.assertRaises(ValueError):
            ControllerEventFactory.create_event(relay_status_event_dict)
        with self.assertRaises(ValueError):
            ControllerEventFactory.create_event(device_status_event_dict)

    def test_controller_event_factory_with_unknown_event_raises_value_error(self):
        """ Test if ValueError is raised when unknown event is passed to ControllerEventFactory

        """
        event_xml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"\
                    "<event>"\
                    "<time type=\"datetime\">2014-11-28T15:30:09Z</time>"\
                    "<type>UnknownEvent</type>"\
                    "<info>"\
                    "<actuator>ALIVE</actuator>"\
                    "</info>"\
                    "</event>"

        event_dict = xmltodict.parse(event_xml)
        with self.assertRaises(ValueError):
            ControllerEventFactory.create_event(event_dict)

    def test_controller_event_invalid_device_status_raises_value_error(self):
        """ Test if ValueError is raised when dict with invalid device status is passed to
        ControllerEventFactory

        """
        # Invalid sensor status in device_status_xml
        device_status_xml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"\
                            "<event>"\
                            "<time type=\"datetime\">2014-11-28T15:30:09Z</time>"\
                            "<type>DeviceStatus</type>"\
                            "<info>"\
                            "<Sensor>AWAY</Sensor>"\
                            "<Actuator>DEAD</Actuator>"\
                            "</info>"\
                            "</event>"

        device_status_event_dict = xmltodict.parse(device_status_xml)
        with self.assertRaises(ValueError):
            ControllerEventFactory.create_event(device_status_event_dict)

    def test_controller_event_with_invalid_relay_status_raises_value_error(self):
        """ Test if ValueError is raised when dict with invalid relay status is passed to
        ControllerEventFactory

        """
        # Invalid relay_1 status in relay_status_xml
        relay_status_xml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"\
                           "<event>"\
                           "<time type=\"datetime\">2014-11-28T15:30:09Z</time>"\
                           "<type>RelayStatus</type>"\
                           "<info>"\
                           "<Relay_1>"\
                           "<mode>AUTO</mode>"\
                           "<status>AWAY</status>"\
                           "</Relay_1>"\
                           "<Relay_2>"\
                           "<mode>AUTO</mode>"\
                           "<status>ON</status>"\
                           "</Relay_2>"\
                           "</info>" \
                           "</event>"

        relay_status_event_dict = xmltodict.parse(relay_status_xml)
        with self.assertRaises(ValueError):
            ControllerEventFactory.create_event(relay_status_event_dict)

    def test_controller_event_with_invalid_relay_mode_raises_value_error(self):
        """ Test if ValueError is raised when dict with invalid relay mode is passed to
        ControllerEventFactory

        """
        # Invalid relay_1 mode in relay_status_xml
        relay_status_xml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"\
                           "<event>"\
                           "<time type=\"datetime\">2014-11-28T15:30:09Z</time>"\
                           "<type>RelayStatus</type>"\
                           "<info>"\
                           "<Relay_1>"\
                           "<mode>VIRTUAL</mode>"\
                           "<status>ON</status>"\
                           "</Relay_1>"\
                           "<Relay_2>"\
                           "<mode>AUTO</mode>"\
                           "<status>ON</status>"\
                           "</Relay_2>"\
                           "</info>" \
                           "</event>"

        relay_status_event_dict = xmltodict.parse(relay_status_xml)
        with self.assertRaises(ValueError):
            ControllerEventFactory.create_event(relay_status_event_dict)

if __name__ == '__main__':
    unittest.main()
