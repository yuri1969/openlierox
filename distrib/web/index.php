<html>
<head><title>OpenLieroX</title></head>

<?php
	// read_from_file
	include_once("fileio.php");
?>

<body>
<p>This is the homepage of <b>OpenLieroX</b>.</p>
<p>
	<h2>Description</h2>
	OpenLierox is an extremely addictive realtime worms shoot-em-up backed
	by an active gamers community.<br>
	Dozens of levels and mods are available to provide endless gaming pleasure.
</p>
<p>
	<h2>About</h2>
	The original game was coded by Jason Boettcher and later
	released under the zlib-licence.<br>
	This version is based on it, ported to Linux and a lot enhanced
	by Dark Charlie and Albert Zeyer.
</p>
<p>
	<h2>Screenshots</h2>
	<img src="lierox3.png"><br><br>
	<img src="lierox31.png">
</p>
<p>
	<h2>Downloads</h2>
	<a href="http://sourceforge.net/project/showfiles.php?group_id=180059">
	Download from SourceForge-mirros</a><br>
	<br>
	<b>0.57_beta1</b> (released 2007-01-27)<br>
	<a href="tarball/OpenLieroX_0.57_beta1.src.tar.bz">OpenLieroX Source tar.bz</a><br>	
	<a href="tarball/OpenLieroX_0.57_beta1.src.zip">OpenLieroX Source zip</a><br>
	<a href="ebuild/games-action/openlierox/openlierox-0.57_beta1.ebuild">OpenLieroX Gentoo ebuild</a><br>
	(Feel free to post any success-stories on Gentoo
	at <a href="http://bugs.gentoo.org/show_bug.cgi?id=164009">this topic</a>,
	related to the ebuild, on the Gentoo-Bugtracker.)<br>
	<br>
	<b>Levels and mods</b> (you need them for playing online)<br>
	<a href="additions/lx0.56_pack1.9.zip">LieroX 0.56 Pack 1.9</a><br>
	<a href="additions/another_lx_pack_2007_01_05.zip">another LX Pack (2007-01-05)</a><br>
	<br>
<?php 
	$VERSION = read_from_file("VERSION");
?>
	<b>current <?php echo $VERSION; ?></b></br>
	<a href="tarball/OpenLieroX_<?php echo $VERSION; ?>.src.tar.bz">OpenLieroX Source tar.bz</a><br>
	<a href="tarball/OpenLieroX_<?php echo $VERSION; ?>.src.zip">OpenLieroX Source zip</a><br>
	<a href="tarball/OpenLieroX_<?php echo $VERSION; ?>.win32.zip">OpenLieroX Win32 binary zip</a><br>
	<br>
	<b>other Download-sources</b><br>
	Take also a look in the 
	<a href="http://lxalliance.net/smf/index.php/topic,3071.0.html">
	OpenLieroX-related topic</a> on the biggest LieroX-forum. You will find
	other Windows-releases and you can discuss anything about the game there.
</p>
<p>
	<h2>General installation hints</h2>
	If you are not using Gentoo (where this will be done automatically
	within the ebuild), go downloading also the Liero packs.
	You need them, if you want to play online, because they are widely used.<br>
	Extract them into ~/.OpenLieroX or /usr/share/OpenLieroX.
</p>
<p>
	<h2>Installation under Gentoo</h2>
	Download the provided ebuild and install it.<br>
	For example, you can do this by (bad but simple way):<br>
	<pre>
		su -
		cd /usr/portage
		mkdir -p games-action/openlierox
		cd games-action/openlierox
		wget http://openlierox.sourceforge.net/games-action/openlierox/openlierox-0.57_beta1.ebuild
		echo "games-action/openlierox ~x86" >> /etc/portage/package.keywords
		FEATURES="-strict" emerge openlierox
	</pre>
</p>
<p>
	<h2>Installation somewhere</h2>
	Download the source and extract it. Take a look into the file
	<i>DEPS</i> for the information, which dependencies are needed.
	Install the missing dependencies.<br>
	Then use the <i>compile.sh</i> to
	compile it. If you want to install it into your system, use the
	<i>install.sh</i>. Take a look at these both scripts, if you want
	information about environment-variables you can use.<br>
	Use the start.sh script, if you don't want to install it.<br>
	For example:<br>
	<pre>
		./compile.sh
		./start.sh
	</pre>
</p>
<p>
	<h2>Installation under Debian/Ubuntu</h2>
	Follow the installation @somewhere. You have only one problem:
	HawkNL doesn't exist for Debian/Ubuntu. But there is the possibility
	to compile OpenLieroX with HawkNL builtin. Simply do (after you have
	checked the file DEPS for other needed packages, you have to install
	with 'sudo apt-get install PACKAGE'):
	<pre>
		HAWKNL_BUILTIN=1 ./compile.sh
	</pre>
</p>
<p>
	<h2>Details</h2>
	The game uses case insensitive filenames (it will use the first 
	found on case sensitive filesystems).<br>
	The game searches the paths ~/.OpenLieroX, 
	./ and /usr/share/OpenLieroX
	(or under Gentoo: /usr/share/games/OpenLieroX)
	for game-data (all path are relativ to this bases) (in this 
	order). You can also
	add more searchpathes in cfg/options.cfg (you also can change 
	the searchpath-order here). Own modified configs,
	screenshots and other stuff always will be stored in 
	~/.OpenLieroX.
</p>
<p>
	<h2>Contact</h2>
	If you like to write any comments, bug reports or anything else,
	use the following mail-adress for now:<br>
	<i>openlierox [at] az2000 [dot] de</i>
</p>
<p>
	<h2>Links</h2>
	<a href="http://sourceforge.net/projects/openlierox/">SourceForge project-site</a><br>
	<a href="http://lxalliance.net/lierox/">official LieroX site</a><br>
	<a href="http://www.az2000.de/">Alberts homepage</a>
</p>
</body>
</html>
