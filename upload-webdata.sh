#!/bin/bash
#
# Copyright 2018 Google Inc. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

set -e

if [[ -z "$APP_INSTANCE_NAME" ]]; then
  echo "Define APP_INSTANCE_NAME environment variable!"
  exit 1
fi

if [[ -z "$CONTAINER_NAME" ]]; then
  echo "Define CONTAINER_NAME environment variable!"
  exit 1
fi

echo "Performing upload of content to compute instances..."
gcloud compute scp --zone=us-east1-b --recurse blog "$APP_INSTANCE_NAME":~

echo "Performing copy of content to container"
gcloud compute ssh --zone=us-east1-b "$APP_INSTANCE_NAME" -- "docker cp blog/. $CONTAINER_NAME:/usr/src/thaihuynh.xyz/blog"

echo "Upload operation finished."
