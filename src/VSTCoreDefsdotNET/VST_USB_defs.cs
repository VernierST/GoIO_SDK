/*********************************************************************************

Copyright (c) 2010, Vernier Software & Technology
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of Vernier Software & Technology nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL VERNIER SOFTWARE & TECHNOLOGY BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

**********************************************************************************/

using System;
using System.Collections.Generic;
using System.Text;

namespace VSTCoreDefsdotNET
{
	public struct VST_USB_defs
	{
		/// <summary>
		/// Vernier Software and Technology USB vendor id
		/// </summary>
		public const Int32 VENDOR_ID = 0x08F7;
		public const Int32 PRODUCT_ID_LABPRO = 0x0001;
		public const Int32 PRODUCT_ID_GO_TEMP = 0x0002;
		public const Int32 PRODUCT_ID_GO_LINK = 0x0003;
		public const Int32 PRODUCT_ID_GO_MOTION = 0x0004;
		public const Int32 PRODUCT_ID_LABQUEST = 0x0005;
		public const Int32 PRODUCT_ID_SPECTROVIS = 0x0006;
		public const Int32 PRODUCT_ID_MINI_GC = 0x0007;
		public const Int32 PRODUCT_ID_LABQUEST_MINI = 0x0008;
		public const Int32 PRODUCT_ID_SPECTROVIS_PLUS = 0x0009;
	}
}
