""" Tests for FlowException, FlowCoreException and FlowMessagingException class
"""
import sys
import os
sys.path.append(os.path.join(os.path.dirname(__file__), *([os.path.pardir] * 1)))
import unittest
from libflow.flow import FlowCoreException, FlowMessagingException, FlowException


class TestFlowExceptions(unittest.TestCase):
    """ Testing of classes related to flow exceptions
    """
    def test_flow_exception_expected(self):
        """ Test for FlowException class
        """
        flow_exception = FlowException("error message")
        self.assertEqual(flow_exception.message, "error message")

    def test_flow_core_exception_expected(self):
        """ Test for FlowCoreException class
        """
        flow_exception = FlowCoreException("Allocation failed", 2)
        self.assertEqual(flow_exception.error_code, 2)
        self.assertEqual(flow_exception.message, "Allocation failed caused by error code 2: Memory:"
                                                 " there is not enough memory to fulfill this "
                                                 "operation")

        flow_exception = FlowCoreException("No error", 100)
        self.assertEqual(flow_exception.message, "No error caused by error code 100: Error msg for"
                                                 " code: 100 is not available")

    def test_flow_messaging_exception_expected(self):
        """ Test for FlowMessagingException class
        """
        flow_exception = FlowMessagingException("cannot call api", 6)
        self.assertEqual(flow_exception.error_code, 6)
        self.assertEqual(flow_exception.message, "cannot call api caused by error code 6: Method"
                                                 " unavailable")

        flow_exception = FlowMessagingException("No error", 100)
        self.assertEqual(flow_exception.message, "No error caused by error code 100: Error msg for"
                                                 " code: 100 is not available")

if __name__ == '__main__':
    unittest.main()
