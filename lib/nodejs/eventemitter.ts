/**
 * @author mrdoob / http://mrdoob.com/
 * @author Jesús Leganés Combarro "Piranna" <piranna@gmail.com>
 */

export class EventEmitter {
    constructor(private subject?: any) {
        if (!this.subject)
            this.subject = this;
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

            const dummyListener = this.subject['on' + event.type];
            if (typeof dummyListener === 'function') {
                listeners.add(dummyListener);
            }

            listeners.forEach(listener => {
                if (typeof listener === 'object' && typeof listener.handleEvent === 'function') {
                    listener.handleEvent(event);
                } else {
                    listener.call(this.subject, event);
                }
            });
        });

        return true;
    }

    removeEventListener(type, listener) {
        const listeners = this._listeners = this._listeners || {};
        if (listeners[type]) {
            listeners[type].delete(listener);
        }
    }
}