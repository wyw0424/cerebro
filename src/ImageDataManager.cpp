#include "ImageDataManager.h"


ImageDataManager::ImageDataManager(): STASH_DIR( "/tmp/cerebro_stash" )
{
    //
    // $ rm -rf ${save_dir}
    // $ mkdir ${save_dir}
    // #include <cstdlib>
    string system_cmd;

    system_cmd = string("rm -rf "+STASH_DIR).c_str();
    cout << TermColor::YELLOW() << "[ImageDataManager::ImageDataManager] " << system_cmd << TermColor::RESET() << endl;
    const int rm_dir_err = RawFileIO::exec_cmd( system_cmd );

    if ( rm_dir_err == -1 )
    {
        cout << TermColor::RED() << "[ImageDataManager::ImageDataManager] Error removing directory!\n" << TermColor::RESET() << endl;
        exit(1);
    }

    system_cmd = string("mkdir -p "+STASH_DIR).c_str();
    cout << TermColor::YELLOW() << "[ImageDataManager::ImageDataManager] " << system_cmd << TermColor::RESET() << endl;
    // const int dir_err = system( system_cmd.c_str() );
    const int dir_err = RawFileIO::exec_cmd( system_cmd );
    if ( dir_err == -1 )
    {
        cout << TermColor::RED() << "[ImageDataManager::ImageDataManager] Error creating directory!\n" << TermColor::RESET() << endl;
        exit(1);
    }
    // done emptying the directory.
}

ImageDataManager::~ImageDataManager()
{
    std::lock_guard<std::mutex> lk(m);
    // TODO
    // rm all available on RAM
    status.clear();
    image_data.clear();

    // rm -rf stash folder
    string system_cmd;

    system_cmd = string("rm -rf "+STASH_DIR).c_str();
    cout << TermColor::YELLOW() << "[ImageDataManager::ImageDataManager] " << system_cmd << TermColor::RESET() << endl;
    const int rm_dir_err = RawFileIO::exec_cmd( system_cmd );

    if ( rm_dir_err == -1 )
    {
        cout << TermColor::RED() << "[ImageDataManager::ImageDataManager] Error removing directory!\n" << TermColor::RESET() << endl;
        exit(1);
    }

}

bool ImageDataManager::setImage( const string ns, const ros::Time t, const cv::Mat img )
{
    cout << "[ImageDataManager::setImage] not implemented\n";
    exit(1);
}

// #define __ImageDataManager__setImageFromMsg( msg ) msg;
#define __ImageDataManager__setImageFromMsg( msg ) ;
bool ImageDataManager::setNewImageFromMsg( const string ns, const sensor_msgs::ImageConstPtr msg )
{
    std::lock_guard<std::mutex> lk(m);
    cv::Mat __image = (cv_bridge::toCvCopy(msg)->image).clone();


    auto key = std::make_pair(ns, msg->header.stamp);
    __ImageDataManager__setImageFromMsg(
    cout << TermColor::iWHITE() << "[ImageDataManager::setImageFromMsg] insert at(" << ns << "," << msg->header.stamp << ")" << TermColor::RESET() << endl;
    )

    // TODO: throw error if attempting to over-write
    status[ key ] = MEMSTAT::AVAILABLE_ON_RAM;
    image_data[ key ] = __image;
    return true;
}


// #define __ImageDataManager__getImage( msg ) msg;
#define __ImageDataManager__getImage( msg ) ;
bool ImageDataManager::getImage( const string ns, const ros::Time t, cv::Mat& outImg ) const
{
    std::lock_guard<std::mutex> lk(m);

    auto key = std::make_pair(ns,t);
    if( status.count(key) > 0 ) {
        if( status.at(key) == MEMSTAT::AVAILABLE_ON_RAM ) {
            __ImageDataManager__getImage(
            cout << TermColor::iGREEN() << "[ImageDataManager::getImage] retrived (from ram) at ns=" << ns << " t=" << t << TermColor::RESET() << endl;
            )
            outImg = image_data.at( key );
            return true;
        }
        else {
            if( status.at(key) == MEMSTAT::AVAILABLE_ON_DISK )
            {
                const string fname = key_to_imagename(ns, t);
                __ImageDataManager__getImage(
                cout << TermColor::iYELLOW() << "[ImageDataManager::getImage] retrived (fname=" << fname << ") at ns=" << ns << " t=" << t << ".\nthis is a quickfix implementation better way is to implement a caching way. TODO." << TermColor::RESET() << endl;
                )
                outImg = cv::imread( fname );
                if( !outImg.data )
                {
                    cout << TermColor::RED() << "[ImageDataManager::getImage] failed to load image " << fname << TermColor::RESET() << endl;
                    exit(1);
                    return false;
                }
                return true;
            }

            if( status.at(key) == MEMSTAT::UNAVAILABLE )
            {
                cout << TermColor::iYELLOW() << "[ImageDataManager::getImage] WARNING you requested image which was MEMSTAT::UNAVAILABLE, this should not be happening but for now return false,  at ns=" << ns << " t=" << t << TermColor::RESET() << endl;
                return false;
            }


            cout <<  TermColor::iGREEN() <<  "[ImageDataManager::getImage] from disk retrival not implemented\n" << TermColor::RESET();
            exit(1);
        }
    }
    else
    {
        cout << TermColor::RED() << "[ImageDataManager::getImage] FATAL-ERROR you requested image ns=" << ns << ", t=" << t << "; However it was not found on the map. FATAL ERRR\n" << TermColor::RESET();
        exit(1);
        return false;
    }

    return false;
}


// #define __ImageDataManager__rmImage(msg) msg;
#define __ImageDataManager__rmImage(msg) ;
bool ImageDataManager::rmImage( const string ns, const ros::Time t )
{
    std::lock_guard<std::mutex> lk(m);
    auto key = std::make_pair(ns, t);
    if( status.count(key) > 0  )
    {
        if( status.at(key) == MEMSTAT::AVAILABLE_ON_RAM ) {
            __ImageDataManager__rmImage(
            cout << TermColor::iWHITE() << "[ImageDataManager::rmImage] ns=" << ns << ", t=" << t << ". FOUND, available on ram, do complete removal" << TermColor::RESET() << endl;
            )
            status[key] = MEMSTAT::UNAVAILABLE;

            auto it_b = image_data.find( key );
            image_data.erase( it_b );
            return true;
        }
        else {
            __ImageDataManager__rmImage(
            cout << TermColor::iWHITE() << "[ImageDataManager::rmImage] ns=" << ns << ", t=" << t << ".";
            cout << "FOUND, however its status is UNAVAILABLE or AVAILABLE_ON_DISK, this means it was previously removed or stashed. No action to take now." << TermColor::RESET() << endl;
            )
            return false;
        }

    }
    else
    {
        cout << TermColor::RED() << "[ImageDataManager::rmImage] FATAL-ERROR you requested to remove ns=" << ns << ", t=" << t << "; However it was not found on the map. FATAL ERRR\n" << TermColor::RESET();
        exit(1);
        return false;
    }

}

// #define __ImageDataManager__stashImage(msg) msg;
#define __ImageDataManager__stashImage(msg) ;
bool ImageDataManager::stashImage( const string ns, const ros::Time t )
{
    std::lock_guard<std::mutex> lk(m);
    __ImageDataManager__stashImage(
    cout << TermColor::iCYAN() << "[ImageDataManager::stashImage] ns=" << ns << ", t=" << t << TermColor::RESET() << endl;
    )
    auto key = std::make_pair( ns, t );
    if( status.count( key ) > 0 )
    {
        if( status.at(key) == MEMSTAT::AVAILABLE_ON_RAM )
        {
            // save to disk
            auto it_b = image_data.find( key );
            // string sfname = STASH_DIR+"/"+ns+"__"+to_string(t.toNSec())+".jpg";
            string sfname = key_to_imagename( ns, t );
            ElapsedTime elp; elp.tic();
            __ImageDataManager__stashImage(
            cout << TermColor::iCYAN() << "[ImageDataManager] imwrite(" << sfname  << ")\t elapsed_time_ms=" << elp.toc_milli() << TermColor::RESET() << endl ;
            )
            cv::imwrite( sfname, it_b->second );

            // erase from map
            image_data.erase( it_b );

            // change status
            status[key] = MEMSTAT::AVAILABLE_ON_DISK;
            return true;
        } else {
            __ImageDataManager__stashImage(
            cout << TermColor::iCYAN() << "[ImageDataManager::stashImage] warning status is not AVAILABLE_ON_RAM so do nothing. You asked me to stash an image which doesnt exists on RAM.\n" << TermColor::RESET();
            )
            return false;
        }
    }
    else
    {
        cout << TermColor::RED() << "[ImageDataManager::stashImage] FATAL-ERROR you requested to stash ns=" << ns << ", t=" << t << "; However it was not found on the map. FATAL ERRR\n" << TermColor::RESET();
        exit(1);
    }
}

bool ImageDataManager::isImageRetrivable( const string ns, const ros::Time t ) const
{
    std::lock_guard<std::mutex> lk(m);
    auto key = std::make_pair( ns, t );
    if( status.count( key ) > 0 ) {
        if( status.at( key ) == MEMSTAT::AVAILABLE_ON_RAM || status.at( key ) == MEMSTAT::AVAILABLE_ON_DISK ) {
            return true;
        }
        else {
            return false;
        }
    }
    else {
        return false;
    }
}

// _ImageDataManager_printstatus_cout cout
#define _ImageDataManager_printstatus_cout myfile
bool ImageDataManager::print_status( string fname )
{
    std::lock_guard<std::mutex> lk(m);

    ofstream myfile;
    myfile.open (fname);

    int AVAILABLE_ON_RAM=0, AVAILABLE_ON_DISK=0, UNAVAILABLE=0;
    for( auto it=status.begin() ; it!=status.end() ; it++ )
    {
        if( it->second == MEMSTAT::AVAILABLE_ON_RAM ) {
            _ImageDataManager_printstatus_cout << TermColor::iGREEN() << " " << TermColor::RESET();
            AVAILABLE_ON_RAM++;
        }

        if( it->second == MEMSTAT::AVAILABLE_ON_DISK ) {
            _ImageDataManager_printstatus_cout << TermColor::iYELLOW() << " " << TermColor::RESET();
            AVAILABLE_ON_DISK++;
        }

        if( it->second == MEMSTAT::UNAVAILABLE ) {
            _ImageDataManager_printstatus_cout << TermColor::iMAGENTA() << " " << TermColor::RESET();
            UNAVAILABLE++;
        }


    }
    _ImageDataManager_printstatus_cout << endl;
    _ImageDataManager_printstatus_cout <<  TermColor::iGREEN() << "AVAILABLE_ON_RAM=" << AVAILABLE_ON_RAM << " " << TermColor::RESET() ;
    _ImageDataManager_printstatus_cout << TermColor::iYELLOW() << "AVAILABLE_ON_DISK=" << AVAILABLE_ON_DISK << " " << TermColor::RESET();
    _ImageDataManager_printstatus_cout << TermColor::iMAGENTA() << "UNAVAILABLE=" << UNAVAILABLE << " " << TermColor::RESET();
    _ImageDataManager_printstatus_cout << "TOTAL=" << AVAILABLE_ON_RAM+AVAILABLE_ON_DISK+UNAVAILABLE << " ";
    _ImageDataManager_printstatus_cout << endl;

    myfile.close();
    return true;
}
