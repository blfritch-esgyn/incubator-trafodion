/**
* @@@ START COPYRIGHT @@@

Licensed to the Apache Software Foundation (ASF) under one
or more contributor license agreements.  See the NOTICE file
distributed with this work for additional information
regarding copyright ownership.  The ASF licenses this file
to you under the Apache License, Version 2.0 (the
"License"); you may not use this file except in compliance
with the License.  You may obtain a copy of the License at

  http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing,
software distributed under the License is distributed on an
"AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
KIND, either express or implied.  See the License for the
specific language governing permissions and limitations
under the License.

* @@@ END COPYRIGHT @@@
 */
package org.trafodion.dcs.master.listener;

import java.sql.SQLException;
import java.io.*;
import java.nio.*;
import java.nio.channels.*;
import java.nio.channels.spi.*;
import java.net.*;
import java.util.*;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

class UserDesc  {
	private static  final Log LOG = LogFactory.getLog(UserDesc.class);

	int userDescType;
	byte[] userSid;
	String domainName;
	String userName;
	String password;

	void extractFromByteBuffer(ByteBuffer buf) throws java.io.UnsupportedEncodingException{
		userDescType = buf.getInt();
		userSid = Util.extractByteArray(buf);
		domainName = Util.extractString(buf);
		userName = Util.extractString(buf);
		password = Util.extractString(buf);
	}
}
