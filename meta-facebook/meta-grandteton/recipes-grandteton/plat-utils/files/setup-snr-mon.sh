#!/bin/bash
#
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This program file is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation; version 2 of the License.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
# for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program in a file named COPYING; if not, write to the
# Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor,
# Boston, MA 02110-1301 USA

### BEGIN INIT INFO
# Provides:          setup-por
# Required-Start:
# Required-Stop:
# Default-Start:     S
# Default-Stop:
# Short-Description: Power on Server
### END INIT INFO

PATH=/sbin:/bin:/usr/sbin:/usr/bin:/usr/local/bin

# shellcheck disable=SC1091
. /usr/local/bin/openbmc-utils.sh
. /usr/local/fbpackages/utils/ast-functions


#MB_HSC_MODULE="1"  # ltc4282/ltc4286
kv get mb_hsc_module
rev=$?

if [ $rev -eq 0 ];
then
  sed -i "2{s/$/ hsc/}" /etc/sv/sensord/run
fi

#SWB_HSC_MODULE="1"  # ltc4282/ltc4286
kv get swb_hsc_module
rev=$?

if [ $rev -eq 0 ];
then
  sed -i "2{s/$/ swb_hsc/}" /etc/sv/sensord/run
fi
