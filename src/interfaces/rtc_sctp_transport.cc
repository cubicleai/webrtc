/**
 * Copyright (c) 2022 Astronaut Labs, LLC. All rights reserved.
 * Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "src/interfaces/rtc_sctp_transport.h"

#include "src/converters/absl.h"
#include "src/converters/napi.h"
#include "src/enums/webrtc/sctp_transport_state.h"
#include "src/interfaces/rtc_dtls_transport.h"
#include "src/interfaces/rtc_peer_connection/peer_connection_factory.h"

namespace node_webrtc {

Napi::FunctionReference& RTCSctpTransport::constructor() {
  static Napi::FunctionReference constructor;
  return constructor;
}

RTCSctpTransport::RTCSctpTransport(const Napi::CallbackInfo& info)
  : AsyncObjectWrapWithLoop<RTCSctpTransport>("RTCSctpTransport", *this, info) {
  if (info.Length() != 2 || !info[0].IsObject() || !info[1].IsExternal()) {
    Napi::TypeError::New(info.Env(), "You cannot construct an RTCSctpTransport").ThrowAsJavaScriptException();
    return;
  }

  auto factory = PeerConnectionFactory::Unwrap(info[0].ToObject());
  auto transport = *info[1].As<Napi::External<rtc::scoped_refptr<webrtc::SctpTransportInterface>>>().Data();

  _factory = factory;
  _factory->Ref();

  _transport = std::move(transport);

  _factory->_workerThread->Invoke<void>(RTC_FROM_HERE, [this]() {
    _dtls_transport = _transport->dtls_transport();
    _transport->RegisterObserver(this);
  });

  if (_transport->Information().state() == webrtc::SctpTransportState::kClosed) {
    Stop();  // NOLINT
  }
}

void RTCSctpTransport::Finalize(Napi::Env env) {
  Napi::HandleScope scope(PeerConnectionFactory::constructor().Env());
  _factory->Unref();
  _factory = nullptr;
  wrap()->Release(this);
}

void RTCSctpTransport::Stop() {
  _transport->UnregisterObserver();
  AsyncObjectWrapWithLoop<RTCSctpTransport>::Stop();
}

Wrap <
RTCSctpTransport*,
rtc::scoped_refptr<webrtc::SctpTransportInterface>,
PeerConnectionFactory*
> * RTCSctpTransport::wrap() {
  static auto wrap = new node_webrtc::Wrap <
  RTCSctpTransport*,
  rtc::scoped_refptr<webrtc::SctpTransportInterface>,
  PeerConnectionFactory*
  > (RTCSctpTransport::Create);
  return wrap;
}

RTCSctpTransport* RTCSctpTransport::Create(
    PeerConnectionFactory* factory,
    rtc::scoped_refptr<webrtc::SctpTransportInterface> transport) {
  auto env = constructor().Env();
  Napi::HandleScope scope(env);

  auto object = constructor().New({
    factory->Value(),
    Napi::External<rtc::scoped_refptr<webrtc::SctpTransportInterface>>::New(env, &transport)
  });

  auto unwrapped = Unwrap(object);

  // The reference is owned by RTCPeerConnection
  unwrapped->Ref();
  return unwrapped;
}

void RTCSctpTransport::OnStateChange(const webrtc::SctpTransportInformation info) {
  Dispatch(CreateCallback<RTCSctpTransport>([this]() {
    auto env = Env();
    Napi::HandleScope scope(env);
    auto event = Napi::Object::New(env);
    event.Set("type", Napi::String::New(env, "statechange"));
    MakeCallback("dispatchEvent", { event });
  }));

  if (info.state() == webrtc::SctpTransportState::kClosed) {
    Stop();
  }
}

Napi::Value RTCSctpTransport::GetTransport(const Napi::CallbackInfo&) {
  return RTCDtlsTransport::wrap()->GetOrCreate(_factory, _dtls_transport)->Value();
}

Napi::Value RTCSctpTransport::GetState(const Napi::CallbackInfo& info) {
  CONVERT_OR_THROW_AND_RETURN_NAPI(info.Env(), _transport->Information().state(), result, Napi::Value)
  return result;
}

Napi::Value RTCSctpTransport::GetMaxMessageSize(const Napi::CallbackInfo& info) {
  CONVERT_OR_THROW_AND_RETURN_NAPI(info.Env(), _transport->Information().MaxMessageSize(), result, Napi::Value)
  return result;
}

Napi::Value RTCSctpTransport::GetMaxChannels(const Napi::CallbackInfo& info) {
  CONVERT_OR_THROW_AND_RETURN_NAPI(info.Env(), _transport->Information().MaxChannels(), result, Napi::Value)
  return result;
}

void RTCSctpTransport::Init(Napi::Env env, Napi::Object exports) {
  auto func = DefineClass(env, "RTCSctpTransport", {
    InstanceAccessor("transport", &RTCSctpTransport::GetTransport, nullptr),
    InstanceAccessor("state", &RTCSctpTransport::GetState, nullptr),
    InstanceAccessor("maxMessageSize", &RTCSctpTransport::GetMaxMessageSize, nullptr),
    InstanceAccessor("maxChannels", &RTCSctpTransport::GetMaxChannels, nullptr)
  });

  constructor() = Napi::Persistent(func);
  constructor().SuppressDestruct();

  exports.Set("RTCSctpTransport", func);
}

}  // namespace node_webrtc
