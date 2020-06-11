/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

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

