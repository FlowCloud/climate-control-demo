""" Admin app entry point
"""
import sys
import os
sys.path.append(os.path.join(os.path.dirname(__file__), os.path.pardir))
from admin import main

if __name__ == "__main__":
    main.main()
