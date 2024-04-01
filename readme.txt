ExtProtect - Extrarius's Warcraft 3 Map Protector Version 0.1.2.0ß

New versions will be available for download at http://extprotect.psychosanity.com/

Please Send Information On Any Bugs To ExtProtect@PsychoSanity.com

==============
 Instructions
==============

1) Run ExtProtect.exe

2) Click on the ... button by the Map Name box to select a map to protect.

3) Type your password into the Password box.

4) Type it again in the confirm box to make sure you didn't make any typos.

5) Click on the ... button by the Save As box to choose where to save the modified map.

6) Click the button labeled "Protect" or "UnProtect" to do it.


While it is protecting/unprotecting, the status box between the about
and protect/unprotect button will say "Protecting" or "UnProtecting".
When the operation is complete, it will say "Done". If there was an
error and the operation couldn't be completed, it will display the
word "Error".

*Note: If the button says "Not A Map" or "No Backup", you can't modify the map
       because its either not a valid warcraft 3 map or it wasnt protected with
       ExtProtect and doesn't have a backup stored inside the map.


=========
 Options
=========

After running the program for the first time, it will create a file called
ExtProtect.ini that contains some settings. Two of the settings you don't
need to mess with - they store the last used filenames, but two others you
might want to change.

If you like typing in the filename instead of using the browse box you can
change "ReadOnlyFileNames" to false.

Also, you can change "WarnNoBackup" to false and the program won't warn you
anymore that leaving the password fields blank means no backup is stored.


==============================================
 Acknowledgements (People Whose Stuff I Used)
==============================================

Dr B. R. Gladman for his implementation of the Rijndael encryption algorithm. (Obtained on Artifact Desktop)

RSA Data Security, Inc. for their MD5 Message-Digest Algorithm and the implementation. (http://www.rsasecurity.com/)

Shadow Flare, for her SFMPQAPI library. (http://shadowflare.ancillaediting.net/)

Markus Oberhumer and Laszlo Molnar for their Ultimate Packer for eXecutables. (http://upx.sourceforge.net)


==========================
 Copyright And Disclaimer
==========================

ExtProtect is Copyright 2004 by Extrarius (ExtProtect@PsychoSanity.com)
Redistribution and use without modification are permitted.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


=================
 Changes History
=================

June 28, 2004 - Fixed several bugs with the jass obfscation and greatly increased run speed (Beta 0.1.2.0)

February 4, 2004 - Fixed another bug with jass obfuscation that caused some maps to become non-functional (Beta 0.1.1.1)

January 1, 2004 - Fixed a bug with jass obfuscation that caused some maps to be non-functional after protection(Beta 0.1.1.0)

September 24, 2003 - Fixed a bug with jass obfuscation that caused 'variable names' inside strings to be erraneously changed(Beta 0.1.0.7)

September 24, 2003 - Fixed a bug that caused some maps to be slightly larger than needed after protection and some major issues with the jass obfuscation (Beta 0.1.0.6)

September 23, 2003 - Fixed a bug that caused jass script to not be properly obfuscated (Beta 0.1.0.5)

March 9, 2003 - Changed The Protection In An Attempt To Fix a Bug Preventing Sound From Working On Some Maps (Beta 0.1.0.4) [Not Sure Bug Was Fixed, The Person Reporting The Bug Never Sent Me The Map Demonstrating The Bug]

February 15, 2003 - Fixed A Bug That Caused Some Maps Not To Open In WE After Being UnProtected (Beta 0.1.0.3)

December 15, 2002 - Fixed A Bug Introduced By The Code Changes From 0.1.0.1 (Beta 0.1.0.2)

December 14, 2002 - Opening A Protected Map No Longer Crashes WE (Beta 0.1.0.1)

December 14, 2002 - First Release (Beta 0.1.0.0)