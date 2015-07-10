""" setup script for climate_control project
    "python setup.py sdist"
    "python setup.py build"
    "python setup.py install"

"""

from distutils.command.build_py import build_py as _build_py
from distutils.command.sdist import sdist as _sdist
from distutils.command.install import install as _install
from distutils.version import StrictVersion
from subprocess import call
from climate_control.common.miscellaneous import get_app_version
from setuptools import setup, find_packages
from importlib import import_module
from pkg_resources import get_distribution


def generate_ui_files():
    """ Generates python file from the Qt .ui and .qrc file

    """
    print "Generating UI files"
    call(["pyside-uic", "climate_control/common/ui/login.ui", "-o",
          "climate_control/common/ui/login.py"])
    call(["pyside-uic", "climate_control/admin/mainwindow.ui", "-o",
          "climate_control/admin/mainwindow.py"])
    call(["pyside-uic", "climate_control/display/mainwindow.ui", "-o",
          "climate_control/display/mainwindow.py"])
    call(["pyside-rcc", "climate_control/common/ui/resource.qrc", "-o",
          "climate_control/resource_rc.py"])


class ClimateControlBuild(_build_py):
    """ Inherited distutils build_py class for generating UI files

    """
    def run(self):
        generate_ui_files()
        _build_py.run(self)


class ClimateControlSdist(_sdist):
    """ Inherited distutils sdist class for generating UI files

    """
    def run(self):
        generate_ui_files()
        _sdist.run(self)


class ClimateControlInstall(_install):
    """ Inherited distutils install class for checking if required packages are installed

    """
    def run(self):
        required_packages = [("PySide", "1.2.1"), ("matplotlib", "1.3.1"),
                             ("netifaces", "0.8")]
        packages_not_found = []

        for package, version in required_packages:
            try:
                module = import_module(package)
                try:
                    if StrictVersion(module.__version__) < StrictVersion(version):
                        packages_not_found.append((package, version))
                except AttributeError:
                    # some packages like netifaces don't have __version__ attribute set,
                    # try using pkg_resource
                    if StrictVersion(get_distribution(package).version) < StrictVersion(version):
                        packages_not_found.append((package, version))
            except ImportError:
                packages_not_found.append((package, version))

        if packages_not_found:
            print "Please install the following packages manually before trying to install " \
                  "this package"
            for package, version in packages_not_found:
                print "* {} >= {}".format(package, version)
        else:
            _install.run(self)


_, EXTERNAL_VERSION = get_app_version()


setup(name="climate_control",
      version=EXTERNAL_VERSION,
      description="PC based applications for administration and display of temperature & humidity readings in the climate-control-demo.",
      author="Imagination Technologies",
      packages=find_packages(),
      license="FlowCloud License",
      package_data={
          "climate_control": [
              "version.txt",
              "libflow/flow_config.cnf"]},
      provides=["climate_control"],
      install_requires=["enum34 >= 1.0.4",
                        "xmltodict >= 0.9.2"],
      cmdclass={"build_py": ClimateControlBuild,
                "sdist": ClimateControlSdist,
                'install': ClimateControlInstall})
