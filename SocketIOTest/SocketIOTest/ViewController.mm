//
//  ViewController.m
//  SocketIOTest
//
//  Created by James Chen on 4/11/17.
//  Copyright © 2017 James Chen. All rights reserved.
//

#import "ViewController.h"

#include "Emitter.h"
#include "EngineIOParser.h"

void test()
{
    Emitter e;
    e.on("hello", [](const Value& value){
        const ValueArray& v = value.asArray();
        for (auto& e : v)
        {
            std::string str = e.asString();
            printf("111hello event...: %s\n", str.c_str());
        }
    }, 111);

    e.on("hello", [](const Value& value){
        const ValueArray& v = value.asArray();
        for (auto& e : v)
        {
            std::string str = e.asString();
            printf("222hello event...: %s\n", str.c_str());
        }
    }, 222);

    e.emit("hello", Value("world"));
    e.emit("hello", Value("world"));
    e.emit("hello", Value("world"));
    e.off("hello", 111);
    e.emit("hello", Value("world"));

    std::string s = "97:0{\"sid\":\"TtSjLPMbakR18eXMAAAE\",\"upgrades\":[\"websocket\"],\"pingInterval\":25000,\"pingTimeout\":60000}";
    engineio::parser::decodePayload(s);
}

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];

    // Do any additional setup after loading the view.
    test();
}


- (void)setRepresentedObject:(id)representedObject {
    [super setRepresentedObject:representedObject];

    // Update the view, if already loaded.
}


@end
