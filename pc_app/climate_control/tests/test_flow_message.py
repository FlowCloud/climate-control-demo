""" Tests for FlowMessage class
"""
import sys
import os
sys.path.append(os.path.join(os.path.dirname(__file__), *([os.path.pardir] * 1)))
import unittest
from mock import Mock


class TestFlowMessage(unittest.TestCase):
    """ Testing of FlowMessage class members
    """
    def setUp(self):
        self.flow_message = Mock()
        self.flow_message.get_message_content.return_value = "hi"

    def test_get_message_content(self):
        """ Test to check get_message_content()
        """
        self.assertEqual(self.flow_message.get_message_content(), "hi")


if __name__ == '__main__':
    unittest.main()
