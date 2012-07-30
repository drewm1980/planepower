#include <dc1394/dc1394.h>
#include <libraw1394/raw1394.h>

#include <iostream>

//compile with -ldc1394_control

// or is it -ldc1394


//found: svn export https://libdc1394.svn.sourceforge.net/svnroot/libdc1394/trunk/libdc1394/ -r 488

using namespace std;

// see dc1394_reset_bus in examples of libdc1394


/*
dc1394error_t dc1394_cleanup_iso_channels_and_bandwidth(dc1394camera_t *camera) {
  dc1394camera_priv_t * cpriv = DC1394_CAMERA_PRIV (camera);
  platform_camera_t * craw = cpriv->pcam;
  int i;

  if (craw->capture_is_set>0)

   return DC1394_CAPTURE_IS_RUNNING;

  // free all iso channels 

  for (i=0;i<DC1394_NUM_ISO_CHANNELS;i++)

    raw1394_channel_modify(craw->handle, i, RAW1394_MODIFY_FREE);

  // free bandwidth
  raw1394_bandwidth_modify(craw->handle, 4915, RAW1394_MODIFY_FREE);
  return DC1394_SUCCESS;

}
*/


int reinsert_cameras(void)
{
	
    dc1394camera_t * camera;
    dc1394error_t err;
    dc1394_t * d;
    dc1394camera_list_t * list;

    d = dc1394_new ();                                                     /* Initialize libdc1394 */
    if (!d)
        return 1;

    err=dc1394_camera_enumerate (d, &list);                                /* Find cameras */
    cout << "found " << list->num << " cameras" << endl;
    cout << "I will properly reinsert them" << endl;



    for(unsigned int iX=0; iX<list->num; iX++) {
		camera = dc1394_camera_new (d, list->ids[iX].guid); 
		dc1394_reset_bus (camera);
		dc1394_camera_free (camera);  
    }

    dc1394_camera_free_list (list);
    dc1394_free (d);
    return 0;
}

int main () {
	return reinsert_cameras();
}
