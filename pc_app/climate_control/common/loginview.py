""" Login dialog shared between admin and display app

"""

from PySide.QtGui import QCompleter, QDialog
from PySide.QtCore import Qt, Signal
from .ui.login import Ui_LoginDialog


# pylint: disable=too-many-public-methods
class LoginDialog(QDialog, Ui_LoginDialog):
    """ Login dialog

    Allows user to login
    """
    close_signal = Signal()

    def __init__(self, user_names=None, version=None, parent=None):
        super(LoginDialog, self).__init__(parent)
        self.setupUi(self)
        self.screen_label.setScaledContents(True)
        if version:
            self.setWindowTitle("{} v{}".format(self.windowTitle(), version))
        self.setFocus()
        self.setAttribute(Qt.WA_DeleteOnClose)
        # Show previously entered user names
        completer = QCompleter(user_names)
        self.user_name.setCompleter(completer)

    def closeEvent(self, *dummy_args, **dummy_kargs):
        self.close_signal.emit()

# pylint: enable=too-many-public-methods
