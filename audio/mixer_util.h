/*
 * Copyright (C) 2018 The LineageOS Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef MIXER_UTIL_H
#define MIXER_UTIL_H

#define MIXER_PATH_DEFAULT          "/system/etc/mixer_paths_0.xml"
#define MIXER_PATH_MAX_LENGTH 100
#define MAX_PCMDAI_LINKS 6

int init_pcm_ids(const char *filename);

#endif
