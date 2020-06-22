/* 
 * File:   WindowCallback.h
 * Author: Michał Żelechowski <MichalZelechowski@github.com>
 *
 * Created on 11 czerwca 2020, 17:17
 */

#ifndef WINDOWCALLBACK_H
#define WINDOWCALLBACK_H

namespace zvlk {

    class WindowCallback {
    public:
        virtual void resize(int width, int height) = 0;

    };
}
#endif /* WINDOWCALLBACK_H */

