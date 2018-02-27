//
// Adafruit invests time and resources providing this open source code.
// Please support Adafruit and open source hardware by purchasing
// products from Adafruit!
//
// Copyright (c) 2015-2016 Adafruit Industries
// Authors: Tony DiCola, Todd Treece
// Licensed under the MIT license.
//
// All text above must be included in any redistribution.
//
#include "AdafruitIO_Block.h"
#include "AdafruitIO.h"
#include "AdafruitIO_Dashboard.h"
#include "WiFiClientSecure.h"

AdafruitIO_Block::AdafruitIO_Block(AdafruitIO_Dashboard *d, AdafruitIO_Feed *f)
{
  _dashboard = d;
  _feed = f;
}

AdafruitIO_Block::~AdafruitIO_Block(){}

String AdafruitIO_Block::properties()
{
  String props = "{}";
  return props;
}

String AdafruitIO_Block::dimensions()
{
  String dim = "\",\"size_x\":\"";
  dim += _width();
  dim += "\",\"size_y\":\"";
  dim += _height();

  if(_row() > 0) {
    dim += "\",\"row\":\"";
    dim += _row();
  }

  if(_column() > 0) {
    dim += "\",\"column\":\"";
    dim += _column();
  }

  return dim;
}

const char* AdafruitIO_Block::type()
{
  return _visual_type;
}

// [CGB]
bool AdafruitIO_Block::exists(const char *name)
{
  WiFiClientSecure client;
  const char*  server = "io.adafruit.com";
  int Response;
  String line;

  String url = "/api/v2/";
  url += _dashboard->user();
  url += "/dashboards/";
  url += _dashboard->name;
  url += "/blocks";

  // Convert feed name to lower case and replace spaces with '-'
  url.toLowerCase();
  url.replace(" ", "-");

  if(!client.connect(server, 443)) return(false);
  else 
  {
    client.printf("GET %s HTTP/1.1\n", url.c_str());
    client.printf("X-AIO-Key: %s\n", _dashboard->io()->_key);
    client.print("Host: ");
    client.println(server);
    client.println("Connection: close");
    client.println();
    Response = 0;
  
    // Read headers 
    while(client.connected())
    {
      line = client.readStringUntil('\n');
      if(line.indexOf("HTTP/1.1") != -1)
      {
        line = line.substring(8);
        Response = line.toInt();
      }
      if(line == "\r") break;
    }

    // Read content body
	line = "";
    while(client.connected())
    {
      while(client.available())
	  {
        char c = client.read();
		line += c;
	  }
	}
  }

  // Close connection
  client.stop();
  
  // Good response?
//  Serial.printf("Block exist = %d (%s)\n", Response, line.c_str());
  if(Response != 200) return(false);
  
  // Scan block looking for "name":"xxxxx"
  url = "\"name\":\"" + String(name) + "\"";
  url.toLowerCase();
  url.replace(" ", "-");
  line.toLowerCase();
  return((line.indexOf(url) == -1 ? false : true));
}

// [CGB]
bool AdafruitIO_Block::save(const char *n)
{
  String url = "/api/v2/";
  url += _dashboard->user();
  url += "/dashboards/";
  url += _dashboard->name;
  url += "/blocks";
  url.toLowerCase();
  url.replace(" ", "-");

  String block_feeds = "[{\"feed_id\":\"";
  block_feeds += _feed->name;
  block_feeds += "\"}]";
  block_feeds.toLowerCase();
  block_feeds.replace(" ", "-");

  String body = "{\"visual_type\":\"";
  body += type();
  body += dimensions();
  body += "\",\"name\":\"";
  body += String(n);
  body += "\",\"properties\":";
  body += properties();
  body += ",\"block_feeds\":";
  body += block_feeds;
  body += "}";
//  body.toLowerCase();
//  body.replace(" ", "-");

  WiFiClientSecure client;
  const char*  server = "io.adafruit.com";
  int Response;
  String line;
  
  if(!client.connect(server, 443)) return(false);
  else 
  {
    client.printf("POST %s HTTP/1.1\n", url.c_str());
    client.println("Content-Type: application/json");
    client.printf("Content-Length: %d\n", body.length());
    client.printf("X-AIO-Key: %s\n", _dashboard->io()->_key);
    client.print("Host: ");
    client.println(server);
    client.println("Connection: close");
    client.println();
	client.println(body);
	client.println();
    Response = 0;
  
    // Read headers 
    while(client.connected())
    {
      line = client.readStringUntil('\n');
      if(line.indexOf("HTTP/1.1") != -1)
      {
        line = line.substring(8);
        Response = line.toInt();
      }
      if(line == "\r") break;
    }

    // Read content body
    while(client.connected())
    {
      while(client.available())
	  {
        char c = client.read();
	  }
	}
  }

  // Close connection
  client.stop();
//  Serial.printf("Block create = %d (%s)\n", Response, body.c_str());
  return(Response == 200);
  
/*  
  HttpClient *http = _dashboard->io()->_http;

  http->beginRequest();
  http->post(url.c_str());

  http->sendHeader("Content-Type", "application/json");
  http->sendHeader("Content-Length", body.length());
  http->sendHeader("X-AIO-Key", _dashboard->io()->_key);

  // the following call to endRequest
  // should be replaced by beginBody once the
  // Arduino HTTP Client Library is updated
  // http->beginBody();
  http->endRequest();

  http->print(body);
  http->endRequest();

  int status = http->responseStatusCode();
  http->responseBody(); // needs to be read even if not used

  return status == 200;
*/  
}
