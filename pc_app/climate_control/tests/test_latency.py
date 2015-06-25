""" Tests for Latency class
"""
import sys
import os
sys.path.append(os.path.join(os.path.dirname(__file__), *([os.path.pardir] * 1)))
import unittest
from common.model import Latency


class TestLatency(unittest.TestCase):
    """ Testing of Latency class members
    """
    def setUp(self):
        self.latency = Latency()

    def test_calculate_exponential_moving_average_with_one_value(self):
        """ Test to check calculate_exponential_moving_average() with 1 value
        """
        self.assertEqual(self.latency.calculate_exponential_moving_average(10), 10)

    def test_calculate_exponential_moving_average_with_hundred_values(self):
        """ Test to check calculate_exponential_moving_average() with 100 values
        """
        for i in range(1, 100, 1):
            self.latency.calculate_exponential_moving_average(i)

        self.assertEqual("{:.2f}".format(self.latency.exponential_moving_average), '94.44')

if __name__ == '__main__':
    unittest.main()
