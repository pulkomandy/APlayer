void Convert(char *fileName);

int main(void)
{
	// PolyKit
	Convert("../PolyKit/Sources/BeOS/PolyKit");

	// Server
	Convert("../APlayer/Server/BeOS/Server");

	// Clients
	Convert("../APlayer/Clients/MainWindowSystem/BeOS/MainWindowSystem");

	// Players
	Convert("../APlayer/Players/AHX/BeOS/AHX");
	Convert("../APlayer/Players/Fred/BeOS/Fred");
	Convert("../APlayer/Players/FutureComposer/BeOS/FutureComposer");
	Convert("../APlayer/Players/JamCracker/BeOS/JamCracker");
	Convert("../APlayer/Players/MikMod/BeOS/MikMod");
	Convert("../APlayer/Players/ModTracker/BeOS/ModTracker");
	Convert("../APlayer/Players/Mpg123/BeOS/Mpg123");
	Convert("../APlayer/Players/OctaMED/BeOS/OctaMED");
	Convert("../APlayer/Players/OggVorbis/BeOS/OggVorbis");
	Convert("../APlayer/Players/Oktalyzer/BeOS/Oktalyzer");
	Convert("../APlayer/Players/Sample/BeOS/Sample");
	Convert("../APlayer/Players/Sawteeth/BeOS/Sawteeth");
	Convert("../APlayer/Players/SidPlay/BeOS/SidPlay");
	Convert("../APlayer/Players/SoundFX/BeOS/SoundFX");
	Convert("../APlayer/Players/SoundMonitor/BeOS/SoundMonitor");
	Convert("../APlayer/Players/TFMX/BeOS/TFMX");

	// Agents
	Convert("../APlayer/Agents/Decruncher/BeOS/Decruncher");
	Convert("../APlayer/Agents/DiskSaver/BeOS/DiskSaver");
	Convert("../APlayer/Agents/MediaKit/BeOS/MediaKit");
	Convert("../APlayer/Agents/MikModConverter/BeOS/MikModConverter");
	Convert("../APlayer/Agents/ModuleConverter/BeOS/ModuleConverter");
	Convert("../APlayer/Agents/ProWizard/BeOS/ProWizard");
	Convert("../APlayer/Agents/Reverb/BeOS/Reverb");
	Convert("../APlayer/Agents/Scope/BeOS/Scope");
	Convert("../APlayer/Agents/SpinSquare/BeOS/SpinSquare");

	// Converters
	Convert("../APlayer/Converters/AudioIFF/BeOS/AudioIFF");
	Convert("../APlayer/Converters/IFF-16SV/BeOS/IFF-16SV");
	Convert("../APlayer/Converters/IFF-8SVX/BeOS/IFF-8SVX");
	Convert("../APlayer/Converters/Raw/BeOS/Raw");
	Convert("../APlayer/Converters/RIFF-WAVE/BeOS/RIFF-WAVE");

	return (0);
}



void Convert(char *fileName)
{
	char source[255];
	char destination[255];
	BFile sourceFile, destFile;
	BResources res;

	printf("%s\n", fileName);

	// Build the source file name
	strcpy(source, fileName);
	strcat(source, "_x86.rsrc");

	// Build the destination file name
	strcpy(destination, fileName);
	strcat(destination, "_PPC.rsrc");

	// Open the destination file
	if (destFile.SetTo(destination, B_READ_WRITE | B_CREATE_FILE | B_ERASE_FILE) != B_OK)
	{
		printf("Can't open destination file %s.\n", destination);
		return;
	}

	// Set the resource file
	if (res.SetTo(&destFile) != B_OK)
	{
		printf("Can't set the resource file.\n");
		return;
	} 

	// Open the x86 resource file
	if (sourceFile.SetTo(source, B_READ_ONLY) != B_OK)
	{
		printf("Can't open resource file %s.\n", source);
		return;
	}

	// Merge the source file together with the new destination file
	if (res.MergeFrom(&sourceFile) != B_OK)
	{
		printf("Can't merge source file with destination file.\n");
		return;
	}

	// Write it back to disk
	if (res.Sync() != B_OK)
	{
		printf("Can't write the resource to disk.\n");
		return;
	}

	// Change the file type
	BNodeInfo nodeInfo;

	// Set the node info to the node
	if (nodeInfo.SetTo(&destFile) != B_OK)
	{
		printf("Can't set the file type\n");
		return;
	}

	// Change the mime string
	if (nodeInfo.SetType("application/x-be-resource") != B_OK)
	{
		printf("Can't set the file type\n");
		return;
	}
}
