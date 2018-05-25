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

#define LOG_TAG "mixer_util"
#define LOG_NDEBUG 0

#include <errno.h>
#include <stdio.h>
#include <expat.h>
#include <cutils/log.h>
#include <audio_hw.h>
#include <math.h>
#include <mixer_util.h>
#include <samsung_audio.h>


/* init with default ids, but updated while parsing mixer xml */
int pcm_device_table[AUDIO_USECASE_MAX][2] = {
    [USECASE_AUDIO_PLAYBACK] = {SOUND_PLAYBACK_DEVICE, SOUND_PLAYBACK_DEVICE},
    [USECASE_AUDIO_PLAYBACK_OFFLOAD] = {-1, -1},
    [USECASE_AUDIO_PLAYBACK_DEEP_BUFFER] = {SOUND_DEEP_BUFFER_DEVICE, SOUND_DEEP_BUFFER_DEVICE},
    [USECASE_AUDIO_CAPTURE] = {SOUND_CAPTURE_DEVICE, SOUND_CAPTURE_DEVICE},
    [USECASE_BT_SCO] = {SOUND_PLAYBACK_SCO_DEVICE, SOUND_PLAYBACK_SCO_DEVICE},
    [USECASE_FM_RADIO] = {-1, -1},
    [USECASE_VOICE_CALL] = {SOUND_PLAYBACK_VOICE_DEVICE, SOUND_CAPTURE_VOICE_DEVICE},
};

static const char * const uc_pcmdai_table[AUDIO_USECASE_MAX] = {

    [USECASE_AUDIO_PLAYBACK] = "playback_link",
    [USECASE_AUDIO_PLAYBACK_MULTI_CH] = "none",
    [USECASE_AUDIO_PLAYBACK_OFFLOAD] = "playback_offload_link",
    [USECASE_AUDIO_PLAYBACK_DEEP_BUFFER] = "playback_deep_link",
    [USECASE_AUDIO_CAPTURE] = "capture_link",
    [USECASE_BT_SCO] = "bluetooth_link",
    [USECASE_FM_RADIO] = "fmradio_link",
    [USECASE_VOICE_CALL] = "baseband_link",
};

int get_pcm_device_id(audio_usecase_t usecase, usecase_type_t type)
{
    if (type == PCM_PLAYBACK)
        return pcm_device_table[usecase][0];
    else
        return pcm_device_table[usecase][1];
}

static void process_pcmdai(const XML_Char **attr)
{
    int i;

    for (i=0; i < AUDIO_USECASE_MAX; i++) {
         if (strcmp(attr[0], uc_pcmdai_table[i])==0) {
             ALOGV("%s: found %s = %d", __func__,uc_pcmdai_table[i],atoi((char *)attr[1]));
             pcm_device_table[i][0] = atoi((char *)attr[1]);
             pcm_device_table[i][1] = atoi((char *)attr[1]);
         }
    }

    /* extended capture links, usually equal with same playback links*/

    if (strcmp(attr[0], "baseband_capture_link")==0) {
        ALOGV("%s: found %s id =%d", __func__,"baseband_capture_link",atoi((char *)attr[1]));
        pcm_device_table[USECASE_VOICE_CALL][1] = atoi((char *)attr[1]);
    }

    if (strcmp(attr[0], "bluetooth_capture_link")==0) {
        ALOGV("%s: found %s id =%d", __func__,"bluetooth_capture_link",atoi((char *)attr[1]));
        pcm_device_table[USECASE_BT_SCO][1] = atoi((char *)attr[1]);
    }


    return;
}


static void start_tag(void *userdata __unused, const XML_Char *tag_name,
                      const XML_Char **attr)
{

    if (strcmp(tag_name, "pcmdai") == 0) {
        process_pcmdai(attr);
    }
    return;
}

static void end_tag(void *userdata __unused, const XML_Char *tag_name __unused)
{
}

int init_pcm_ids(const char *filename)
{
    XML_Parser      parser;
    FILE            *file;
    int             ret = 0;
    int             bytes_read;
    void            *buf;
    static const uint32_t kBufSize = 1024;
    char   mixer_file_name[MIXER_PATH_MAX_LENGTH]= {0};

    if (filename == NULL) {
        strlcpy(mixer_file_name, MIXER_PATH_DEFAULT, MIXER_PATH_MAX_LENGTH);
    } else {
        strlcpy(mixer_file_name, filename, MIXER_PATH_MAX_LENGTH);
    }

    ALOGV("%s: mixer file name is %s", __func__, mixer_file_name);

    file = fopen(mixer_file_name, "r");

    if (!file) {
        ALOGD("%s: Failed to open %s, using defaults.",
            __func__, mixer_file_name);
        ret = -ENODEV;
        goto done;
    }


    parser = XML_ParserCreate(NULL);
    if (!parser) {
        ALOGE("%s: Failed to create XML parser!", __func__);
        ret = -ENODEV;
        goto err_close_file;
    }

    XML_SetElementHandler(parser, start_tag, end_tag);

    while (1) {
        buf = XML_GetBuffer(parser, kBufSize);
        if (buf == NULL) {
            ALOGE("%s: XML_GetBuffer failed", __func__);
            ret = -ENOMEM;
            goto err_free_parser;
        }

        bytes_read = fread(buf, 1, kBufSize, file);
        if (bytes_read < 0) {
            ALOGE("%s: fread failed, bytes read = %d", __func__, bytes_read);
             ret = bytes_read;
            goto err_free_parser;
        }

        if (XML_ParseBuffer(parser, bytes_read,
                            bytes_read == 0) == XML_STATUS_ERROR) {
            ALOGE("%s: XML_ParseBuffer failed, for %s",
                __func__, mixer_file_name);
            ret = -EINVAL;
            goto err_free_parser;
        }

        if (bytes_read == 0)
            break;
    }

err_free_parser:
    XML_ParserFree(parser);
err_close_file:
    fclose(file);
done:
    return ret;
}
