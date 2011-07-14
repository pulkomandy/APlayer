How to compile

It's very easy to compile the APlayer sources. The only thing you need to do, is to make sure you have downloaded an official source package from Polycode. Also make sure you have the developer kit installed. If you don't have it installed, you can download it for free on the BeBits homepage.

To compile the Mpg123 player on the x86 platform, you need to download and install Nasm. The reason to do so, is because the player contains some assembler code.  Here is a description on how to install Nasm and get it to work:

1. Download Nasm on BeBits (http://www.bebits.com) and install it.
2. Because of a bug in the Nasm installer, create a link in the directory "/boot/develop/BeIDE/tools/" which points to the Nasm binary in "/boot/home/config/bin/nasm".

That should do it. You are now ready to compile APlayer. Now do the following if you're running on a x86 platform:

1. Open a Terminal window and change the directory to <where APlayer sources are stored>/APlayer_4/Master
2. Type make and press enter. This will start the compiler which will compile all projects in APlayer.

If you're running on a PPC platform, do this instead:

1. Open the Master directory in the Tracker.
2. Double click on the Master_PPC.proj file.
3. Select the menu item Make in the Project menu.
4. Wait until it's finish compiling. Ignore the linker errors at the end.

If you compile one of the beta versions of APlayer, the compiled files will be very huge on x86, That's because they contain debugger information.

You should now have a full build in the bin directory. Try to run APlayer. It's stored in the APlayer/bin/BeOS_??? directory.

If you have any problems to compile APlayer, contact us at aplayer@polycode.dk and we will try to help you.

The APlayer Team / Polycode
