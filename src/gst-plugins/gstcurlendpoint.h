#ifndef _GST_CURLENDPOINT_H_
#define _GST_CURLENDPOINT_H_

#include <gst/video/gstvideofilter.h>

G_BEGIN_DECLS
#define GST_TYPE_CURLENDPOINT   (gst_curlendpoint_get_type())
#define GST_CURLENDPOINT(obj)   (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_CURLENDPOINT,Gstcurlendpoint))
#define GST_CURLENDPOINT_CLASS(klass)   (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_CURLENDPOINT,GstcurlendpointClass))
#define GST_IS_CURLENDPOINT(obj)   (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_CURLENDPOINT))
#define GST_IS_CURLENDPOINT_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_CURLENDPOINT))
typedef struct _Gstcurlendpoint Gstcurlendpoint;
typedef struct _GstcurlendpointClass GstcurlendpointClass;
typedef struct _GstcurlendpointPrivate GstcurlendpointPrivate;

struct _Gstcurlendpoint
{
  GstVideoFilter base;
  GstcurlendpointPrivate *priv;
};

struct _GstcurlendpointClass
{
  GstVideoFilterClass base_curlendpoint_class;
};

GType gst_curlendpoint_get_type (void);

gboolean gst_curlendpoint_plugin_init (GstPlugin * plugin);

G_END_DECLS
#endif /* _GST_CURLENDPOINT_H_ */
