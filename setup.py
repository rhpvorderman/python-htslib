# Copyright (C) 2026 Leiden University Medical Center
# This file is part of python-htslib
#
# Python-htslib is free software: you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option) any
# later version.
#
# Python-htslib is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
# details.
#
# You should have received a copy of the GNU General Public License along with
# python-htslib. If not, see <https://www.gnu.org/licenses/>.

from setuptools import Extension, setup

from wheel.bdist_wheel import bdist_wheel


class bdist_wheel_abi3(bdist_wheel):
    def get_tag(self):
        python, abi, plat = super().get_tag()

        if python.startswith("cp"):
            return "cp311", "abi3", plat
        return python, abi, plat

setup(
    ext_modules=[
        Extension(
            "htslib._htslib",
            ["src/htslib/_htslibmodule.c"],
            libraries=["hts"],
            py_limited_api=True
        ),
    ],
    cmdclass={"bdist_wheel": bdist_wheel_abi3},
)
