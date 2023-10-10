#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <espeak-ng/speak_lib.h>
#include "../../utils/strutils.h"

/* required */
#include "../modtape.h"
#include "../../core/modules.h"
#include "../../core/log.h"
#include "../../core/irc.h"

static FILE *fwav;
static int samplerate = 0;
static pthread_mutex_t speak_lock = PTHREAD_MUTEX_INITIALIZER;

static void /* Write4Bytes espeak.cpp */
fput4(FILE *f, int value)
{
	for(int ix = 0; ix < 4; ix++) {
		fputc(value & 0xff,f);
		value = value >> 8;
	}
}

static void /* OpenWavFile espeak.cpp */
wav_open(int rate)
{
	static unsigned char wave_hdr[44] = {
		'R','I','F','F',0x24,0xf0,0xff,0x7f,'W','A','V','E','f','m','t',' ',
		0x10,0,0,0,1,0,1,0,  9,0x3d,0,0,0x12,0x7a,0,0,
		2,0,0x10,0,'d','a','t','a',  0x00,0xf0,0xff,0x7f};
	fwav = fopen("/tmp/h.wav", "wb");
	fwrite(wave_hdr,1,24,fwav);
	fput4(fwav,rate);
	fput4(fwav,rate * 2);
	fwrite(&wave_hdr[32],1,12,fwav);
}

static void /* CloseWavFile espeak.cpp */
wav_close(void)
{
	if(fwav == NULL) {
		return;
	}

	fflush(fwav);
	unsigned int pos = ftell(fwav);

	fseek(fwav,4,SEEK_SET);
	fput4(fwav,pos - 8);

	fseek(fwav,40,SEEK_SET);
	fput4(fwav,pos - 44);

	fclose(fwav);
	fwav = NULL;
}

int
espeak_cb(short *wav, int numsamples, espeak_EVENT *events)
{
	if (!wav) {
		wav_close();
		return 0;
	}

	while(events->type != 0)
	{
		if(events->type == espeakEVENT_SAMPLERATE) {
			samplerate = events->id.number;
		}
		events++;
	}

	if (!fwav) {
		wav_open(samplerate);
	}

	fwrite(wav, 1, numsamples*2, fwav);
	return 0;
}

static void
handle_cmdmsg(msg_info *mi, char *msg)
{
	if (strncmp(msg, "speak ", 6) || strlen(msg) < 7) {
		return;
	}

	/* nothing here is threadsafe */
	pthread_mutex_lock(&speak_lock);
	
	/* initialize espeak */
	espeak_Initialize(AUDIO_OUTPUT_SYNCHRONOUS, BUFSIZE, NULL, 0);
	espeak_SetParameter(espeakPITCH, 100, 0);
	espeak_SetParameter(espeakRATE, 120, 0);
	espeak_SetVoiceByName("en-us+f3");
	espeak_SetSynthCallback(espeak_cb);

	/* generate audio, caught by callback written to /tmp/h.wav */
	espeak_Synth(strchr(msg, ' ')+1, BUFSIZE, 0, 0, 0, espeakCHARS_AUTO,
			(int *){NULL}, (void *){NULL});

	/* upload with systemwide tmpupload util
	 * couldnt be bothered about ffmpeg, sorry */
	FILE *cp = popen("ffmpeg -y -i /tmp/h.wav /tmp/h.mp3 && "
			"~/tmpupload.sh /tmp/h.mp3", "r");
	char tmpurl[BUFSIZE] = {'\0'};
	fgets(tmpurl, BUFSIZE-1, cp);
	pclose(cp);
	pthread_mutex_unlock(&speak_lock);
	send_privmsg("%s", tmpurl);
}

void
speak_init()
{
	mods_new("speak", true);
	mods_cmdmsg_handler(handle_cmdmsg);
}
