#include "connection.h"
#include <dock/dock.h>
#include "mainwindow.h"
#include <QtMultimedia/QSoundEffect>
#include "wavetableconfig.h"
#include "wavetable.h"

bool Connection::initSound ()
{
// 	bool foundSupportedFormat = false;
// 
// 	QList<int> sampleRatesList;
// #ifdef Q_OS_WIN
// 	// The Windows audio backend does not correctly report format support
// 	// (see QTBUG-9100).  Furthermore, although the audio subsystem captures
// 	// at 11025Hz, the resulting audio is corrupted.
// 	sampleRatesList += 8000;
// #endif
// 
// 	sampleRatesList += m_audioOutputDevice.supportedSampleRates();
// 	sampleRatesList = sampleRatesList.toSet().toList(); // remove duplicates
// 	qSort(sampleRatesList);
// 
// 	QList<int> channelsList;
// 	channelsList += m_audioOutputDevice.supportedChannelCounts();
// 	channelsList = channelsList.toSet().toList();
// 	qSort(channelsList);
// 
// 	QAudioFormat format;
// 	format.setByteOrder(QAudioFormat::LittleEndian);
// 	format.setCodec("audio/pcm");
// 	format.setSampleSize(16);
// 	format.setSampleType(QAudioFormat::SignedInt);
// 	int sampleRate, channels;
// 	foreach(sampleRate, sampleRatesList)
// 	{
// 		if (foundSupportedFormat)
// 			break;
// 		format.setSampleRate(sampleRate);
// 		foreach(channels, channelsList)
// 		{
// 			format.setChannelCount(channels);
// 			const bool outputSupport = m_audioOutputDevice.isFormatSupported(format);
// 			if (outputSupport)
// 			{
// 				foundSupportedFormat = true;
// 				break;
// 			}
// 		}
// 	}
// 
// 	if (!foundSupportedFormat)
// 		format = QAudioFormat();
// 
// 
// 	const int    NotifyIntervalMs = 100;
// 	m_audioFormat = format;
// 	m_audioOutput = new QAudioOutput(m_audioOutputDevice, m_audioFormat, this);
// 	m_audioOutput->setNotifyInterval(NotifyIntervalMs);
	return true;
}

void Connection::destroySound ()
{
	//m_wavetable->destroy();
// 	delete m_audioOutput;
// 	m_audioOutput = nullptr;
}


bool Connection::loadWaveTable (WaveTableConfig & cfg)
{
	if (cfg.m_waves.size() == 0)
	{
		{
			WaveConfig wc;
			wc.m_fpath = "C:\\devel\\_trace\\trace_server\\waves\\Tech-01.wav";
			wc.m_name = "test_tech1";
			cfg.m_waves.push_back(wc);
		}
		{
			WaveConfig wc;
			wc.m_fpath = "C:\\devel\\_trace\\trace_server\\waves\\Tech-03.wav";
			wc.m_name = "test_tech3";
			cfg.m_waves.push_back(wc);
		}
		{
			WaveConfig wc;
			wc.m_fpath = "C:\\devel\\_trace\\trace_server\\waves\\Tech-04.wav";
			wc.m_name = "test_tech4";
			cfg.m_waves.push_back(wc);
		}
		{
			WaveConfig wc;
			wc.m_fpath = "C:\\devel\\_trace\\trace_server\\waves\\Tech-05.wav";
			wc.m_name = "test_tech5";
			cfg.m_waves.push_back(wc);
		}
	}
	return true;
}

bool Connection::handleSoundCommand (DecodedCommand const & cmd, E_ReceiveMode mode)
{
	//if (getClosestFeatureState(e_data_plot) == e_FtrDisabled)
	//	return true;

	OCTET_STRING const & m = cmd.choice.snd.msg;
	QString msg = QString::fromLatin1(reinterpret_cast<char const *>(m.buf), m.size);

	int const slash_pos = msg.lastIndexOf(QChar('/'));
	msg.chop(msg.size() - slash_pos);

	float const vol = cmd.choice.snd.vol;
	int const loop = cmd.choice.snd.loop;

	m_wavetable->play(msg, vol, loop);
	return true;
}
 
