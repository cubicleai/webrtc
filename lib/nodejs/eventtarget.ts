/**
 * @author mrdoob / http://mrdoob.com/
 * @author Jesús Leganés Combarro "Piranna" <piranna@gmail.com>
 */

export class EventTarget {
    constructor() {
    }

    private _listeners: Record<string, any> = {};

    addEventListener(type, listener) {
        const listeners = this._listeners = this._listeners || {};

        if (!listeners[type]) {
            listeners[type] = new Set();
        }

        listeners[type].add(listener);
    };

    dispatchEvent(event) {
        let listeners = this._listeners = this._listeners || {};

        event.target = this;

        process.nextTick(() => {
            listeners = new Set(listeners[event.type] || []);

            const dummyListener = this['on' + event.type];
            if (typeof dummyListener === 'function') {
                listeners.add(dummyListener);
            }

            listeners.forEach(listener => {
                if (typeof listener === 'object' && typeof listener.handleEvent === 'function') {
                    listener.handleEvent(event);
                } else {
                    listener.call(this, event);
                }
            });
        });
    }

    removeEventListener(type, listener) {
        const listeners = this._listeners = this._listeners || {};
        if (listeners[type]) {
            listeners[type].delete(listener);
        }
    }
}