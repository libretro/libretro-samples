(*

Copyright (c) 2015-2016, Higor Eurípedes
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice, this
  list of conditions and the following disclaimer in the documentation and/or
  other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*)

library paspong;

{$Mode OBJFPC}

uses
  Math,
  SysUtils;

const
  Font: array [0..7, 0..511] of $0..$1 = (
    (0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0),
    (0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,1,1,0,1,1,0,0,0,1,1,0,1,1,0,0,0,0,0,1,1,0,0,0,0,1,1,0,0,0,1,0,0,0,1,1,1,1,0,0,0,0,0,1,1,0,0,0,0,0,0,0,1,1,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,1,1,0,0,0,0,0,1,1,1,1,0,0,0,0,1,1,1,1,0,0,0,0,0,0,1,1,0,0,0,1,1,1,1,1,1,0,0,0,1,1,1,1,0,0,0,1,1,1,1,1,1,0,0,0,1,1,1,1,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,1,1,1,1,0,0,0,0,1,1,1,1,0,0,0,1,1,1,1,1,0,0,0,0,1,1,1,1,0,0,0,1,1,1,1,1,0,0,0,1,1,1,1,1,1,0,0,1,1,1,1,1,1,0,0,0,1,1,1,1,0,0,0,1,1,0,0,1,1,0,0,1,1,1,1,1,1,0,0,0,0,1,1,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,0,0,0,0,1,1,0,0,0,1,0,0,1,1,0,0,1,1,0,0,0,1,1,1,1,0,0,0,1,1,1,1,1,0,0,0,0,1,1,1,1,0,0,0,1,1,1,1,1,0,0,0,0,1,1,1,1,0,0,0,1,1,1,1,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,0,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,1,1,1,1,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0),
    (0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,1,1,0,1,1,0,0,1,1,1,1,1,1,1,0,0,0,1,1,1,1,0,0,0,1,1,0,0,1,0,0,0,1,1,0,0,1,1,0,0,0,0,1,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,1,1,1,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,1,1,0,0,1,1,0,0,0,1,1,1,0,0,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,0,0,1,1,1,0,0,0,1,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,0,0,1,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,1,1,0,0,1,1,0,0,1,0,0,0,0,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1,1,0,0,1,1,0,1,1,0,0,0,1,1,0,0,0,0,0,0,1,1,1,0,1,1,0,0,1,1,1,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,0,1,0,0,0,1,1,1,1,0,0,0,1,1,0,0,1,1,0,0,0,0,0,0,1,1,0,0,0,1,1,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0),
    (0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,1,0,0,1,0,0,0,1,1,0,1,1,0,0,0,1,0,1,1,0,0,0,0,0,0,0,1,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,1,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,1,1,0,1,1,1,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,1,1,0,0,0,0,1,1,1,1,0,0,0,1,1,1,1,1,0,0,0,1,1,1,1,1,0,0,0,0,0,0,1,1,0,0,0,0,1,1,1,1,0,0,0,0,1,1,1,1,1,0,0,0,0,1,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,1,1,0,0,0,0,0,1,1,1,1,0,0,0,0,0,1,1,0,0,0,0,0,0,0,1,1,0,0,0,1,0,1,1,0,1,0,0,1,1,1,1,1,1,0,0,1,1,1,1,1,0,0,0,1,1,0,0,0,0,0,0,1,1,0,0,1,1,0,0,1,1,1,1,0,0,0,0,1,1,1,1,0,0,0,0,1,1,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1,1,0,0,1,1,1,1,0,0,0,0,1,1,0,0,0,0,0,0,1,1,1,1,1,1,0,0,1,1,1,1,1,1,0,0,1,1,0,0,1,1,0,0,1,1,1,1,1,0,0,0,1,1,0,0,1,1,0,0,1,1,1,1,1,0,0,0,0,1,1,1,1,0,0,0,0,0,1,1,0,0,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,1,0,1,0,0,0,0,1,1,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,1,1,0,0,0,0,1,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1,1,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0),
    (0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,1,1,0,0,0,0,1,1,1,1,0,0,0,0,0,1,0,0,0,0,0,1,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,1,0,1,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,1,1,1,0,1,1,0,0,0,0,1,1,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,1,1,0,0,1,1,0,1,1,0,0,0,0,0,0,0,1,1,0,0,1,1,0,0,1,1,0,0,0,0,1,1,0,0,0,0,1,1,0,0,1,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,1,1,0,0,0,0,1,0,1,1,1,0,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,0,0,0,0,1,1,0,0,1,1,0,0,1,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,1,1,0,1,1,1,0,0,1,1,0,0,1,1,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1,1,0,0,1,1,0,1,1,0,0,0,1,1,0,0,0,0,0,0,1,1,0,1,0,1,0,0,1,1,0,1,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,0,0,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,0,0,0,0,1,1,0,0,0,0,1,1,0,0,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,1,1,1,1,0,0,0,1,1,1,1,0,0,0,0,0,0,0,1,1,0,0,0,0,1,1,0,0,0,0,0,1,1,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0),
    (0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,1,1,0,1,0,0,0,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,1,1,0,0,0,0,0,1,1,0,0,1,1,0,0,0,0,1,1,0,0,0,0,0,1,1,0,0,0,0,0,1,1,0,0,1,1,0,0,1,1,1,1,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,0,1,1,0,0,0,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,0,0,1,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,1,1,0,0,0,0,0,1,1,1,1,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,0,0,1,1,0,0,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,0,0,0,0,1,1,0,0,0,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,0,0,0,0,1,1,0,1,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,0,0,1,1,0,0,0,0,1,1,0,0,1,1,0,0,0,1,1,1,1,0,0,0,1,1,1,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,0,1,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0),
    (0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,1,1,0,0,0,0,1,1,1,1,0,0,0,1,0,0,0,1,1,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,1,1,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,1,1,1,1,0,0,0,1,1,1,1,1,1,0,0,0,1,1,1,1,0,0,0,0,0,0,1,1,0,0,0,0,1,1,1,1,0,0,0,0,1,1,1,1,0,0,0,0,1,1,0,0,0,0,0,0,1,1,1,1,0,0,0,0,1,1,1,1,0,0,0,0,0,1,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,1,1,0,0,0,0,0,1,1,1,1,0,0,0,1,1,0,0,1,1,0,0,1,1,1,1,1,0,0,0,0,1,1,1,1,0,0,0,1,1,1,1,1,0,0,0,1,1,1,1,1,1,0,0,1,1,0,0,0,0,0,0,0,1,1,1,1,1,0,0,1,1,0,0,1,1,0,0,1,1,1,1,1,1,0,0,0,1,1,1,1,0,0,0,1,1,0,0,1,1,0,0,1,1,1,1,1,1,0,0,1,1,0,0,0,1,0,0,1,1,0,0,1,1,0,0,0,1,1,1,1,0,0,0,1,1,0,0,0,0,0,0,0,1,1,1,1,1,0,0,1,1,0,0,1,1,0,0,0,1,1,1,1,0,0,0,0,0,1,1,0,0,0,0,0,1,1,1,1,1,0,0,0,0,1,1,0,0,0,0,1,1,0,0,0,1,0,0,1,1,0,0,1,1,0,0,0,1,1,1,1,0,0,0,1,1,1,1,1,1,0,0,0,1,1,1,1,0,0,0,0,0,0,0,1,1,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0),
    (0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1)
);

type
  TRetroEnvironment = function(cmd: UInt32; Data: Pointer): boolean; cdecl;
  TRetroVideoRefresh = procedure(Data: Pointer; Width: UInt32;
    Height: UInt32; pitch: SizeUInt); cdecl;
  TRetroAudioSample = procedure(left: Int16; right: Int16); cdecl;
  TRetroAudioSampleBatch = function(Data: PInt16;
    frames: SizeUInt): Int16; cdecl;
  TRetroInputPoll = procedure; cdecl;
  TRetroInputState = function(port: UInt32; device: UInt32;
    index: UInt32; id: UInt32): Int16; cdecl;

  PRetroSystemInfo = ^TRetroSystemInfo;

  TRetroSystemInfo = record
    LibraryName: PAnsiChar;
    LibraryVersion: PAnsiChar;
    ValidExtensions: PAnsiChar;
    NeedFullpath: boolean;
    BlockExtract: boolean;
  end;

  TRetroGameGeometry = record
    BaseWidth: UInt32;
    BaseHeight: UInt32;
    MaxWidth: UInt32;
    MaxHeight: UInt32;
    AspectRatio: single;
  end;

  TRetroSystemTiming = record
    FPS: double;
    SampleRate: double;
  end;

  PRetroSystemAvInfo = ^TRetroSystemAvInfo;

  TRetroSystemAvInfo = record
    Geometry: TRetroGameGeometry;
    Timing: TRetroSystemTiming;
  end;

  PRetroGameInfo = ^TRetroGameInfo;

  TRetroGameInfo = record
    Path: PAnsiChar;
    Data: Pointer;
    Size: SizeUInt;
    Meta: PAnsiChar;
  end;

  PTARGB1555 = ^TARGB1555;

  TARGB1555 = bitpacked record
    a: $0..$01;
    b: $0..$1f;
    g: $0..$1f;
    r: $0..$1f;
  end;

  PPlayer = ^TPlayer;

  TPlayer = object
    Id: integer;
    Score: 0..99;
    X, Y, W, H: integer;

    procedure Update();
    procedure Draw();
  end;

  TBall = object
    X, Y, W, H: integer;
    HSpeed, VSpeed: integer;

    procedure Reset();
    procedure Update();
    procedure Draw();
  end;

  TGameState = (gsStart, gsPlaying, gsScore);

const
  Width = 320;
  Height = 180;
  Size = 8;
  Speed = 2;

  // https://en.wikipedia.org/wiki/List_of_video_game_console_palettes#SECAM
  Black: TARGB1555 = (a: 1; b: 0 shr 3; g: 1 shr 3; r: 0 shr 3);
  Blue: TARGB1555 = (a: 1; b: 255 shr 3; g: 36 shr 3; r: 0 shr 3);
  Red: TARGB1555 = (a: 1; b: 122 shr 3; g: 58 shr 3; r: 241 shr 3);
  Magenta: TARGB1555 = (a: 1; b: 255 shr 3; g: 75 shr 3; r: 255 shr 3);
  Green: TARGB1555 = (a: 1; b: 0 shr 3; g: 255 shr 3; r: 126 shr 3);
  Cyan: TARGB1555 = (a: 1; b: 255 shr 3; g: 255 shr 3; r: 121 shr 3);
  Yellow: TARGB1555 = (a: 1; b: 61 shr 3; g: 255 shr 3; r: 252 shr 3);
  White: TARGB1555 = (a: 1; b: 252 shr 3; g: 255 shr 3; r: 254 shr 3);

  JoypadB = 0;
  JoypadSelect = 2;
  JoypadStart = 3;
  JoypadUp = 4;
  JoypadDown = 5;
  JoypadLeft = 6;
  JoypadRight = 7;
  JoypadA = 8;

var
  EnvCb: TRetroEnvironment;
  VideoCb: TRetroVideoRefresh;
  PollCb: TRetroInputPoll;
  InputCb: TRetroInputState;
  Screen: array [0..(Width * Height)] of TARGB1555;
  Player: array [1..2] of TPlayer;
  Ball: TBall;
  GameState: TGameState;

function MakeColor(a, r, g, b: byte): TARGB1555;
var
  c: TARGB1555;
begin
  c.a := integer(a > 1);
  c.r := r shr 3;
  c.g := g shr 3;
  c.b := b shr 3;
  Result := c;
end;

procedure Clear(color: TARGB1555);
var
  i: integer;
begin
  for i := 0 to (Width * Height) do
    Screen[i] := color;
end;

procedure ToScreen(x, y, w, h: PInteger);
var
  left, right, top, bottom: integer;
begin
  left := Max(x^, 0);
  right := Min(Width, x^ + w^);
  top := Max(y^, 0);
  bottom := Min(Height, y^ + h^);

  x^ := left;
  y^ := top;
  w^ := right - left;
  h^ := bottom - top;
end;

procedure DrawText(Text: string; x, y: integer; color: TARGB1555);
var
  fx, fy: integer;
  tx, ty, tw, th: integer;
  left, top: integer;
  c: char;
begin
  for c in upCase(Text) do
  begin
    if (c > ' ') and (c < '`') then
    begin
      tx := x;
      ty := y;
      tw := 8;
      th := 8;

      ToScreen(@tx, @ty, @tw, @th);

      if (tw > 0) and (th > 0) then
      begin
        fy := 0;
        for top := ty to ty + th - 1 do
        begin
          fx := (Ord(c) - 32) * 8;
          for left := tx to tx + tw - 1 do
          begin
            if Font[fy][fx] = 1 then
              Screen[top * Width + left] := color;
            fx += 1;
          end;
          fy += 1;
        end;
      end;
    end;
    x := x + 8;
  end;
end;

procedure DrawTextCentered(Text: string; cols, rows: integer;
  color: TARGB1555);
begin
  DrawText(Text, (Width - Length(Text) * 8) div 2 + cols * 8,
    (Height - 8) div 2 + rows * 8, color);
end;

procedure DrawRect(x, y, w, h: integer; color: TARGB1555);
var
  left, top: integer;
begin
  ToScreen(@x, @y, @w, @h);

  if (w > 0) and (h > 0) then
  begin
    for top := y to y + h - 1 do
    begin
      for left := x to x + w - 1 do
      begin
        Screen[top * Width + left] := color;
      end;
    end;
  end;
end;

// METHODS ---------------------------------------------------------------------

procedure TPlayer.Update();
var
  ymove: integer;
begin
  ymove := InputCb(Id, 1, 0, JoypadDown) - InputCb(Id, 1, 0, JoypadUp);
  Y += ymove;
end;

procedure TPlayer.Draw();
begin
  DrawRect(X, Y, W, H, White);
end;

procedure TBall.Reset();
begin
  X := (Width - W) div 2;
  Y := (Height - H) div 2;
  HSpeed := Speed;
  VSpeed := Speed;

  if Random(100) mod 2 = 0 then
    HSpeed := -HSpeed;
  if Random(100) mod 2 = 0 then
    VSpeed := -VSpeed;
end;

procedure TBall.Update();
var
  i: integer;
begin
  Y += VSpeed;
  if (Y < 0) or (Y + H >= Height) then
    VSpeed := -VSpeed
  else
  begin
    X += HSpeed;

    if (X <= Player[1].X + Player[1].W) and
      ((Y + H >= Player[1].Y) and (Y <= Player[1].Y + Player[1].H)) then
    begin
      HSpeed := -HSpeed;
      X := Player[1].X + Player[1].W;
    end
    else
    if (X + W >= Player[2].X) and ((Y + H >= Player[2].Y) and
      (Y <= Player[2].Y + Player[2].H)) then
    begin
      HSpeed := -HSpeed;
      X := Player[2].X - W;
    end;
  end;
end;

procedure TBall.Draw();
begin
  DrawRect(X, Y, W, H, White);
end;

// LIBRETRO --------------------------------------------------------------------
procedure retro_set_environment(p: TRetroEnvironment); cdecl;
var
  no_game: boolean = True;
begin
  EnvCb := p;
  EnvCb(18, @no_game);
end;

procedure retro_set_video_refresh(p: TRetroVideoRefresh); cdecl;
begin
  VideoCb := p;
end;

procedure retro_set_audio_sample(p: TRetroAudioSample); cdecl;
begin

end;

procedure retro_set_audio_sample_batch(p: TRetroAudioSampleBatch); cdecl;
begin

end;

procedure retro_set_input_poll(p: TRetroInputPoll); cdecl;
begin
  PollCb := p;
end;

procedure retro_set_input_state(p: TRetroInputState); cdecl;
begin
  InputCb := p;
end;

procedure retro_init; cdecl;
begin
  Randomize;
end;

procedure retro_deinit; cdecl;
begin
end;

function retro_api_version: UInt32; cdecl;
begin
  Result := 1;
end;

procedure retro_get_system_info(info: PRetroSystemInfo); cdecl;
begin
  info^.LibraryName := 'pascal test';
  info^.LibraryVersion := '1.0';
end;

procedure retro_get_system_av_info(info: PRetroSystemAvInfo); cdecl;
begin
  with info^.Geometry do
  begin
    BaseWidth := Width;
    BaseHeight := Height;
    MaxWidth := BaseWidth;
    MaxHeight := BaseHeight;
    AspectRatio := single(BaseWidth) / single(BaseHeight);
  end;

  with info^.Timing do
  begin
    FPS := 60;
    SampleRate := 44000;
  end;
end;

procedure retro_set_controller_port_device(port: UInt32;
  device: UInt32); cdecl;
begin
end;

procedure retro_reset; cdecl;
var
  i: integer;
begin
  for i := 1 to 2 do
  begin
    Player[i].Id := 0;//i - 1;
    Player[i].W := Size;
    Player[i].H := Size * 4;
    Player[i].X := (Width - Size) * (i - 1);
    Player[i].Y := (Height - Player[i].H) div 2;
    Player[i].Score := 0;
  end;

  GameState := gsStart;

  Ball.W := Size;
  Ball.H := Ball.W;
  Ball.Reset();
end;

procedure retro_run; cdecl;
var
  i: integer;
begin
  PollCb();

  Clear(Black);

  case GameState of
    gsStart:
    begin
      if InputCb(0, 1, 0, JoypadA) <> 0 then
        GameState := gsPlaying;

      DrawTextCentered('enygmata''s pascal pong', 0, -2, Yellow);
      DrawTextCentered('a: start', 0, 0, Blue);
      DrawTextCentered('b: reset', 0, 1, Blue);
      DrawTextCentered('- press a to start -', 0, 3, Red);
    end;
    gsPlaying:
    begin
      if (InputCb(0, 1, 0, JoypadB) <> 0) or
        (InputCb(0, 1, 0, JoypadB) <> 0) then
        retro_reset()
      else
      begin
        DrawText(Format('%2d/%-2d', [Player[1].Score, Player[2].Score]),
          (Width - 8 * 5) div 2, 8, White);

        for i := 1 to 2 do
        begin
          Player[i].Update();
          Player[i].Draw();
        end;

        Ball.Update();
        Ball.Draw();

        if Ball.X + Ball.W < Player[1].X then
        begin
          Player[2].Score += 1;
          Ball.Reset();
        end
        else if Ball.X > Player[2].X + Player[2].W then
        begin
          Player[1].Score += 1;
          Ball.Reset();
        end;
      end;
    end;
    gsScore:
    begin

    end;
  end;

  VideoCb(@Screen, Width, Height, Width * 2);
end;

function retro_serialize_size: SizeUInt; cdecl;
begin
  Result := 0;
end;

function retro_serialize(Data: Pointer; size: SizeUInt): boolean; cdecl;
begin
  Result := False;
end;

function retro_unserialize(const Data: Pointer;
  size: SizeUInt): boolean; cdecl;
begin
  Result := False;
end;

procedure retro_cheat_reset; cdecl;
begin

end;

procedure retro_cheat_set(index: UInt32; Enabled: boolean;
const code: PAnsiChar); cdecl;
begin

end;

function retro_load_game(const game: PRetroGameInfo): boolean; cdecl;
var
  pixel_format: Int32 = 0; // 0rgb1555
begin
  EnvCb(10, @pixel_format);

  retro_reset;

  Result := True;
end;

function retro_load_game_special(gameType: UInt32; info: PRetroGameInfo;
  numInfo: SizeUInt): boolean; cdecl;
begin
  Result := retro_load_game(info);
end;

procedure retro_unload_game; cdecl;
begin

end;

function retro_get_region: UInt32; cdecl;
begin
  Result := 1;
end;

function retro_get_memory_data(id: UInt32): Pointer; cdecl;
begin
  Result := nil;
end;

function retro_get_memory_size(id: UInt32): SizeUInt; cdecl;
begin
  Result := 0;
end;

exports
  retro_set_environment,
  retro_set_video_refresh,
  retro_set_audio_sample,
  retro_set_audio_sample_batch,
  retro_set_input_poll,
  retro_set_input_state,
  retro_init,
  retro_deinit,
  retro_api_version,
  retro_get_system_info,
  retro_get_system_av_info,
  retro_set_controller_port_device,
  retro_reset,
  retro_run,
  retro_serialize_size,
  retro_serialize,
  retro_unserialize,
  retro_cheat_reset,
  retro_cheat_set,
  retro_load_game,
  retro_load_game_special,
  retro_unload_game,
  retro_get_region,
  retro_get_memory_data,
  retro_get_memory_size;

begin
  writeln();
  writeln('WELCOME TO THE FIRST LIBRETRO GAME WRITTEN IN PASCAL');
  writeln();
end.
