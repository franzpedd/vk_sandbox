# standart modules
import subprocess;
import platform;
import os;
import sys;
import shutil;

# must be installed
import requests;
import tqdm;

# ontain general methods used across the script
class Util :
     
    # downloads a web file, usually an archive
    @classmethod 
    def Download(itslf, url, path) :
        if(os.path.isfile(path) is True) :
            print(f"{path} already downloaded, skipping...");
            return;
        
        request = requests.get(url, stream=True);
        percentage = int(request.headers.get("content-lenght", 0));
        
        with open(path, "wb") as file, tqdm.tqdm(desc = path, total = percentage, unit = "iB", unit_scale = True, unit_divisor = 1024) as bar :
            for data in request.iter_content(chunk_size = 1024) :
                size = file.write(data);
                bar.update(size);
    
    # clones a github repository into the Thirdparty folder
    @classmethod 
    def Clone(itself, url, folder, branch) :
        if(os.path.isdir("Thirdparty") is False) :
            print("Creating Thirdparty folder");
            os.makedirs("Thirdparty");

        if(os.path.isdir(f"Thirdparty/{folder}") is True) :
            print(f"{folder} folder already exists, skipping...");
            return;
        
        scriptDir = os.getcwd();
        os.chdir("Thirdparty");
        subprocess.call(f"git clone {url} {folder}", shell = True);
        os.chdir(folder);
        subprocess.call(f"git checkout {branch}", shell = True);
        os.chdir(scriptDir);

    @classmethod # decompresses an archive
    def Decompress(itself, file, path) :
        if(os.path.isfile(file) is None) :
            print(f"Decompress Error: {file} doesnt exists, skipping...");
            return;
        
        shutil.unpack_archive(file, path);

# holds methods for handle special dependencies
class Project :

    StartingPath = os.getcwd();

    @classmethod # changes the building path, used for moving cmd path when building projects
    def ChangeBuildPath(cls, path) : 
        os.chdir(path);
    
    @classmethod # vulkan renderer api, downloads it's SDK on windows if not already installed (also prompts the installation)
    def DownloadVulkan(itself, version) :
        if(platform.system() == "Windows") :
            if(os.getenv("VULKAN_SDK") is not None) : return;
            if(os.path.isdir("Thirdparty/vulkan-sdk") is False) :  os.makedirs("Thirdparty/vulkan-sdk");

            url = f"https://sdk.lunarg.com/sdk/download/{version}/windows/VulkanSDK-{version}-Installer.exe";
            path = f"Thirdparty/vulkan-sdk/VulkanSDK-{version}-Installer.exe";
            
            Util.Download(url, path);
            os.startfile(os.path.abspath(path));
        
        elif(platform.system() == "Linux") :
            print("Vulkan SDK must be installed as a package, ignore this if already installed");
            return;

        elif(platform.system() == "Darwin") :
            print("Vulkan SDK installation for MacOS is not yet supported, skipping...");
            return;
    
    @classmethod # SDL windowing manager, downloads it's binary instead of compiling it as it's unnecessary work
    def DownloadSDL2(itself, version) :
        if(platform.system() == "Windows") : 
            if(os.path.isdir("Thirdparty/sdl2") is False) :  os.makedirs("Thirdparty/sdl2");

            url = f"https://github.com/libsdl-org/SDL/releases/download/release-{version}/SDL2-devel-{version}-VC.zip";
            path = f"Thirdparty/sdl2/SDL2-devel-{version}-VC.zip";

            Util.Download(url, path);

            if(os.path.isdir("Thirdparty/sdl2/sdl2") is True) : return;
        
            Util.Decompress(path, "Thirdparty/sdl2");
            os.rename(f"Thirdparty/sdl2/SDL2-{version}", "Thirdparty/sdl2/sdl2");

        elif(platform.system() == "Linux") :
            print("SDL must be installed as a package, ignore this if already installed");
            return;

        elif(platform.system() == "Darwin") :
            print("SDL download for MacOS is not yet supported, skipping...");
            return;

    @classmethod # downloads the premake solution generator
    def DownloadPremake(itself, version) :
        if(os.path.isdir("Thirdparty/premake") is False) : os.makedirs("Thirdparty/premake");
        
        url = "";
        path = "";

        if(platform.system() == "Linux") :
            url = f"https://github.com/premake/premake-core/releases/download/v{version}/premake-{version}-linux.tar.gz";
            path = f"Thirdparty/premake/premake-{version}-linux.tar.gz";

        elif(platform.system() == "Windows") :
            url = f"https://github.com/premake/premake-core/releases/download/v{version}/premake-{version}-windows.zip";
            path = f"Thirdparty/premake/premake-{version}-windows.zip";
        
        elif(platform.system() == "Darwin") :
            url = f"https://github.com/premake/premake-core/releases/download/v{version}/premake-{version}-macosx.tar.gz";
            path = f"Thirdparty/premake/premake-{version}-macosx.tar.gz";

        Util.Download(url, path);
        Util.Decompress(path, "Thirdparty/premake");

    @classmethod # generates a solution for the project
    def GenerateSolution(itself) :

        if(len(sys.argv) == 2) :
            subprocess.call(f"../Thirdparty/premake/premake5 {sys.argv[1]}");
            return;
    
        if(platform.system() == "Windows") : subprocess.call(f"../Thirdparty/premake/premake5 vs2022");
        elif(platform.system() == "Linux") :  subprocess.call(f"../Thirdparty/premake/premake5 gmake2", shell = True);
        elif(platform.system() == "Darwin") :  subprocess.call(f"../Thirdparty/premake/premake5 xcode4", shell = True);


## main scope

# download project special dependencies
Project.ChangeBuildPath(os.pardir);
Project.DownloadPremake("5.0.0-beta2");
Project.DownloadVulkan("1.3.236.0");
Project.DownloadSDL2("2.30.2");

# download github dependencies
Util.Clone("https://github.com/g-truc/glm", "glm", "0.9.8");
Util.Clone("https://github.com/zeux/volk", "volk", "master");
Util.Clone("https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator", "vma", "master");
Util.Clone("https://github.com/nothings/stb", "stb", "master");
Util.Clone("https://github.com/ocornut/imgui", "imgui", "docking");
Util.Clone("https://github.com/CedricGuillemet/ImGuizmo", "imgui/imguizmo", "master");

# generates the solution files
Project.ChangeBuildPath(Project.StartingPath);
Project.GenerateSolution();