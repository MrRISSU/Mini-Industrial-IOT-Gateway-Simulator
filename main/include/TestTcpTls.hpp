/******************************************************************************
 * @file    TestTcpTls.cpp
 * @brief   Testing routines for both TCP and TLS transports.
 *
 * Provides standalone test functions to verify the unsecure and secure 
 * transport implementations against a public echo/web server.
 *
 * Author : Huwairis Ibnu Kabeer
 * License: MIT
 ******************************************************************************/
#pragma once

extern void TestTcpTransport();
extern void TestTlsTransport();
extern void TcpTlsTest();

