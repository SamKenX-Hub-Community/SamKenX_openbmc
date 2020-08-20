/**
 * Copyright 2020-present Facebook. All Rights Reserved.
 *
 * This program file is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program in a file named COPYING; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA
 */

package utilities

import (
	"fmt"
	"strings"

	"github.com/facebook/openbmc/tools/flashy/checks_and_remediations/common"
	"github.com/facebook/openbmc/tools/flashy/lib/step"
	"github.com/pkg/errors"
)

func init() {
	step.RegisterUtility(checkImage)
}

func checkImageUsage() {
	fmt.Printf(
		`check-image
-----------
Validates partitions in an image.

Usage: check-image FILE
`)
}

func checkImage(args []string) error {
	if len(args) != 2 || len(args) > 1 && (strings.Contains(args[1], "help") || args[1] == "-h") {
		checkImageUsage()
		return nil
	}

	stepParams := step.StepParams{
		ImageFilePath: args[1],
	}

	err := common.ValidateImagePartitions(stepParams)
	if err != nil {
		return errors.Errorf(err.GetError())
	}

	return nil
}
