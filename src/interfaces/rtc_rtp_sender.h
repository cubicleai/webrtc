/**
 * Copyright (c) 2022 Astronaut Labs, LLC. All rights reserved.
 * Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#pragma once

#include <memory>

#include <webrtc/api/rtp_sender_interface.h>
#include <webrtc/api/scoped_refptr.h>

#include "src/utilities/napi_ref_ptr.h"
#include "src/converters/napi.h"
#include "src/node/async_object_wrap.h"
#include "src/node/wrap.h"
#include "src/interfaces/media_stream_track.h"
#include "src/interfaces/rtc_peer_connection.h"
#include "src/interfaces/media_stream.h"
#include "src/interfaces/rtc_dtls_transport.h"
#include "src/interfaces/rtc_rtp_transceiver.h"

namespace webrtc { class RtpSenderInterface; }

namespace node_webrtc {

	class RTCPeerConnection;
	class RTCRtpTransceiver;

	class RTCRtpSender : public Napi::ObjectWrap<RTCRtpSender> {
	public:
		explicit RTCRtpSender(const Napi::CallbackInfo&);
		static void Init(Napi::Env, Napi::Object);
		static Napi::FunctionReference& constructor();

		static inline void setVideoCapabilities(webrtc::RtpCapabilities caps) {
			videoCapabilities = caps;
		}

		static inline void setAudioCapabilities(webrtc::RtpCapabilities caps) {
			audioCapabilities = caps;
		}

		static RTCRtpSender* Create(
			RTCPeerConnection* pc,
			std::string kind,
			MediaStreamTrack* track,
			std::vector<MediaStream*> streams
		);

		inline void setTrack(MediaStreamTrack* value) {
			track = value;
		}

		inline void setTransport(RTCDtlsTransport* value) {
			transport = value;
		}

		inline void setTransceiver(RTCRtpTransceiver* value) {
			transceiver = value;
		}

		inline RTCRtpTransceiver* getTransceiver() {
			return transceiver;
		}

		inline void setId(std::string value) {
			id = value;
		}

		inline std::string getId() {
			return id;
		}

	private:

		Napi::Value GetTrack(const Napi::CallbackInfo&);
		Napi::Value GetTransport(const Napi::CallbackInfo&);
		Napi::Value GetRtcpTransport(const Napi::CallbackInfo&);

		static Napi::Value GetCapabilities(const Napi::CallbackInfo&);
		static webrtc::RtpCapabilities videoCapabilities;
		static webrtc::RtpCapabilities audioCapabilities;

		Napi::Value GetParameters(const Napi::CallbackInfo&);
		Napi::Value SetParameters(const Napi::CallbackInfo&);
		Napi::Value GetStats(const Napi::CallbackInfo&);
		Napi::Value ReplaceTrack(const Napi::CallbackInfo&);
		Napi::Value SetStreams(const Napi::CallbackInfo&);

		//rtc::scoped_refptr<webrtc::RtpSenderInterface> _sender;

		std::string id;
		napi_ref_ptr<RTCRtpTransceiver> transceiver;
		napi_ref_ptr<RTCPeerConnection> pc;
		std::string kind;
		napi_ref_ptr<MediaStreamTrack> track;
		std::vector<napi_ref_ptr<MediaStream>> streams;
		napi_ref_ptr<RTCDtlsTransport> transport;
	};

	DECLARE_TO_AND_FROM_NAPI(RTCRtpSender*)

}  // namespace node_webrtc
