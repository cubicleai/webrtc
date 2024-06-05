/**
 * Copyright (c) 2022 Astronaut Labs, LLC. All rights reserved.
 * Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "src/interfaces/rtc_rtp_sender.h"

#include <webrtc/api/rtp_parameters.h>

#include "src/converters.h"
#include "src/converters/arguments.h"
#include "src/converters/interfaces.h"
#include "src/converters/null.h"
#include "src/dictionaries/webrtc/rtc_error.h"
#include "src/dictionaries/webrtc/rtp_capabilities.h"
#include "src/dictionaries/webrtc/rtp_parameters.h"
#include "src/enums/webrtc/media_type.h"
#include "src/interfaces/media_stream_track.h"
#include "src/interfaces/media_stream.h"
#include "src/interfaces/rtc_dtls_transport.h"
#include "src/interfaces/rtc_peer_connection/peer_connection_factory.h"
#include "src/node/error_factory.h"
#include "src/node/utility.h"

namespace node_webrtc {
	webrtc::RtpCapabilities RTCRtpSender::audioCapabilities = webrtc::RtpCapabilities();
	webrtc::RtpCapabilities RTCRtpSender::videoCapabilities = webrtc::RtpCapabilities();

	Napi::FunctionReference& RTCRtpSender::constructor() {
		static Napi::FunctionReference constructor;
		return constructor;
	}

	RTCRtpSender::RTCRtpSender(const Napi::CallbackInfo& info): 
		Napi::ObjectWrap<RTCRtpSender>(info) 
	{
		if (info.Length() != 3 || !info[0].IsExternal() || !info[1].IsString() || !info[2].IsExternal()) {
			Napi::TypeError::New(info.Env(), "You cannot construct a RTCRtpSender").ThrowAsJavaScriptException();
			return;
		}

		pc = info[0].As<Napi::External<RTCPeerConnection>>().Data();
		kind = info[1].As<Napi::String>().Utf8Value();
		track = info[2].As<Napi::External<MediaStreamTrack>>().Data();
	}

	Napi::Value RTCRtpSender::GetTrack(const Napi::CallbackInfo& info) {
		if (!track)
			return info.Env().Null();
		return track->Value();
	}

	Napi::Value RTCRtpSender::GetTransport(const Napi::CallbackInfo& info) {
		if (!transport)
			return info.Env().Null();
		return transport->Value();
	}

	Napi::Value RTCRtpSender::GetRtcpTransport(const Napi::CallbackInfo& info) {
		return info.Env().Null();
	}

	Napi::Value RTCRtpSender::GetCapabilities(const Napi::CallbackInfo& info) {
		CONVERT_ARGS_OR_THROW_AND_RETURN_NAPI(info, kindString, std::string)
			if (kindString == "audio" || kindString == "video") {
				auto factory = PeerConnectionFactory::GetOrCreateDefault();
				auto capabilities = kindString == "audio" ? audioCapabilities : videoCapabilities;
				CONVERT_OR_THROW_AND_RETURN_NAPI(info.Env(), capabilities, result, Napi::Value)
					return result;
			}
		return info.Env().Null();
	}

	Napi::Value RTCRtpSender::GetParameters(const Napi::CallbackInfo& info) {
		auto rtcSender = pc->getUnderlying(this);
		auto parameters = rtcSender->GetParameters();
		CONVERT_OR_THROW_AND_RETURN_NAPI(info.Env(), parameters, result, Napi::Value)
			return result;
	}

	Napi::Value RTCRtpSender::SetParameters(const Napi::CallbackInfo& info) {
		CREATE_DEFERRED(info.Env(), deffered)
		CONVERT_ARGS_OR_REJECT_AND_RETURN_NAPI(deferred, info, parameters, webrtc::RtpParameters)

		auto rtcSender = pc->getUnderlying(this);

		if (!rtcSender) {
			Reject(deferred, ErrorFactory::CreateInvalidStateError(info.Env(), "Failed to set parameters"));
			return deferred.Promise();
		}

		// auto parameters = rtcSender->GetParameters();

		auto error = rtcSender->SetParameters(parameters);
		if (error.ok()) {
			deferred.Resolve(info.Env().Undefined());
		} else {
			CONVERT_OR_REJECT_AND_RETURN_NAPI(deferred, &error, reason, Napi::Value)
			deferred.Reject(reason);
		}

		return deferred.Promise();
	}

	Napi::Value RTCRtpSender::GetStats(const Napi::CallbackInfo& info) {
		CREATE_DEFERRED(info.Env(), deffered)
			Reject(deferred, Napi::Error::New(info.Env(), "Not yet implemented; file a feature request against @cubicleai/wrtc"));
		return deferred.Promise();
	}

	Napi::Value RTCRtpSender::ReplaceTrack(const Napi::CallbackInfo& info) {
		CREATE_DEFERRED(info.Env(), deferred)
		CONVERT_ARGS_OR_REJECT_AND_RETURN_NAPI(deferred, info, maybeTrack, Either<Null COMMA MediaStreamTrack*>)
		auto mediaStreamTrack = maybeTrack.FromEither<MediaStreamTrack*>([](auto) {
			return nullptr;
		}, [](auto track) {
			return track;
		});

		auto track = mediaStreamTrack ? mediaStreamTrack->track().get() : nullptr;
		if (track) {
			std::string expectedMediaType = track->kind() == webrtc::MediaStreamTrackInterface::kAudioKind
				? "audio"
				: "video";
			if (this->kind != expectedMediaType) {
				Reject(deferred, Napi::TypeError::New(info.Env(), "Kind does not match").Value().As<Napi::Value>());
				return deferred.Promise();
			}
		}

		if (pc->isClosed()) {
			Reject(deferred, ErrorFactory::CreateInvalidStateError(info.Env(), "The related RTCPeerConnection has been closed."));
			return deferred.Promise();
		}

		auto rtcSender = pc->getUnderlying(this);

		// Likely because the sender has already been removed from the peerconnection.
		if (!rtcSender) {
			Reject(deferred, ErrorFactory::CreateInvalidStateError(info.Env(), "Failed to replaceTrack"));
			return deferred.Promise();
		}

		assert(rtcSender);

		auto result = rtcSender->SetTrack(track);

		if (result) {
			Resolve(deferred, info.Env().Undefined());
			this->track = mediaStreamTrack;
		} else {
			Reject(deferred, ErrorFactory::CreateInvalidStateError(info.Env(), "Failed to replaceTrack"));
		}
		return deferred.Promise();
	}

	Napi::Value RTCRtpSender::SetStreams(const Napi::CallbackInfo& info) {
		auto streams = std::vector<std::string>();
		for (size_t i = 0; i < info.Length(); i++) {
			auto value = info[i];
			auto maybeStream = From<MediaStream*>(value);
			if (maybeStream.IsInvalid()) {
				auto error = maybeStream.ToErrors()[0];
				Napi::TypeError::New(info.Env(), error).ThrowAsJavaScriptException();
				return info.Env().Undefined();
			}
			auto stream = maybeStream.UnsafeFromValid();
			streams.emplace_back(stream->stream()->id());
		}
		pc->getUnderlying(this)->SetStreams(streams);
		return info.Env().Undefined();
	}

	RTCRtpSender* RTCRtpSender::Create(
		RTCPeerConnection* pc,
		std::string kind,
		MediaStreamTrack* track,
		std::vector<MediaStream*> streams
	) {
		auto env = constructor().Env();
		Napi::HandleScope scope(env);

		auto object = constructor().New({
			Napi::External<RTCPeerConnection>::New(env, pc),
			Napi::String::New(env, kind),
			Napi::External<MediaStreamTrack>::New(env, track)
			});

		auto unwrapped = Unwrap(object);
		for (auto stream : streams)
			unwrapped->streams.push_back(stream);

		unwrapped->Ref(); // Owned by RTCPeerConnection
		return unwrapped;
	}

	void RTCRtpSender::Init(Napi::Env env, Napi::Object exports) {
		auto factory = node_webrtc::PeerConnectionFactory::GetOrCreateDefault();
		setAudioCapabilities(factory->factory()->GetRtpSenderCapabilities(cricket::MEDIA_TYPE_AUDIO));
		setVideoCapabilities(factory->factory()->GetRtpSenderCapabilities(cricket::MEDIA_TYPE_VIDEO));

		Napi::Function func = DefineClass(env, "RTCRtpSender", {
		  InstanceAccessor("track", &RTCRtpSender::GetTrack, nullptr),
		  InstanceAccessor("transport", &RTCRtpSender::GetTransport, nullptr),
		  InstanceAccessor("rtcpTransport", &RTCRtpSender::GetRtcpTransport, nullptr),
		  InstanceMethod("getParameters", &RTCRtpSender::GetParameters),
		  InstanceMethod("setParameters", &RTCRtpSender::SetParameters),
		  InstanceMethod("getStats", &RTCRtpSender::GetStats),
		  InstanceMethod("replaceTrack", &RTCRtpSender::ReplaceTrack),
		  InstanceMethod("setStreams", &RTCRtpSender::SetStreams),
		  StaticMethod("getCapabilities", &RTCRtpSender::GetCapabilities)
			});

		constructor() = Napi::Persistent(func);
		constructor().SuppressDestruct();

		exports.Set("RTCRtpSender", func);
	}

	FROM_NAPI_IMPL(RTCRtpSender*, value) {
		return From<Napi::Object>(value).FlatMap<RTCRtpSender*>([](Napi::Object object) {
			auto isRTCRtpSender = false;
			napi_instanceof(object.Env(), object, RTCRtpSender::constructor().Value(), &isRTCRtpSender);
			if (object.Env().IsExceptionPending()) {
				return Validation<RTCRtpSender*>::Invalid(object.Env().GetAndClearPendingException().Message());
			}
			else if (!isRTCRtpSender) {
				return Validation<RTCRtpSender*>::Invalid("This is not an instance of RTCRtpSender");
			}
			return Pure(RTCRtpSender::Unwrap(object));
			});
	}

	TO_NAPI_IMPL(RTCRtpSender*, pair) {
		return Pure(pair.second->Value().As<Napi::Value>());
	}

}  // namespace node_webrtc
