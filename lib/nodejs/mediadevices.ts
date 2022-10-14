import { EventTarget } from "./eventtarget";
import * as native from '../../binding';

export class MediaDevices extends EventTarget {
  enumerateDevices() {
    throw new Error('Not yet implemented; file a feature request for @cubicleai/wrtc');
  };
  
  getSupportedConstraints() {
    throw new Error('Not yet implemented; file a feature request for @cubicleai/wrtc');
  };

  getDisplayMedia(...args) {
    return native.getDisplayMedia(...args);
  }

  getUserMedia(...args) {
    return native.getUserMedia(...args);
  }
}