# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this file,
# You can obtain one at http://mozilla.org/MPL/2.0/.

from __future__ import absolute_import

from setuptools import setup, find_packages

PACKAGE_VERSION = "1.1.0"

# dependencies
deps = ["mozinfo"]

setup(
    name="moztest",
    version=PACKAGE_VERSION,
    description="Package for storing and outputting Mozilla test results",
    long_description="see https://firefox-source-docs.mozilla.org/mozbase/index.html",
    classifiers=[
        "Programming Language :: Python :: 2.7",
        "Programming Language :: Python :: 3",
        "Programming Language :: Python :: 3.5",
        "Development Status :: 5 - Production/Stable",
    ],
    # Get strings from http://pypi.python.org/pypi?%3Aaction=list_classifiers
    keywords="mozilla",
    author="Mozilla Automation and Tools team",
    author_email="tools@lists.mozilla.org",
    url="https://wiki.mozilla.org/Auto-tools/Projects/Mozbase",
    license="MPL",
    packages=find_packages(),
    include_package_data=True,
    zip_safe=False,
    install_requires=deps,
)
