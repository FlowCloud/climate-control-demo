default:
	pyside-uic ./common/ui/login.ui -o ./common/ui/login.py
	pyside-uic ./admin/mainwindow.ui -o ./admin/mainwindow.py
	pyside-uic ./display/mainwindow.ui -o ./display/mainwindow.py
	pyside-rcc ./common/ui/resource.qrc -o ./resource_rc.py

clean:
	rm -f ./common/ui/login.py
	rm -f ./admin/mainwindow.py
	rm -f ./display/mainwindow.py
	rm -f ./resource_rc.py

