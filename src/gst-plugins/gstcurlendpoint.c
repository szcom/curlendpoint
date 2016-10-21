#include "gstcurlendpoint.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gst/gst.h>
#include <kurento/commons/kmsagnosticcaps.h>

#include "gstcurlendpoint.h"

#define PLUGIN_NAME "gstcurlendpoint"

#define APPDATASINK "datasink"
#define APPAUDIOSINK "audiohttpsink"
#define APPVIDEOSINK "videosink"
#define RAW_AUDIO_CAPS "audio/x-raw,channels=1,rate=16000,format=S16LE"

GST_DEBUG_CATEGORY_STATIC (gst_curlendpoint_debug_category);
#define GST_CAT_DEFAULT gst_curlendpoint_debug_category

#define GST_CURLENDPOINT_GET_PRIVATE(obj) ( \
  G_TYPE_INSTANCE_GET_PRIVATE (           \
    (obj),                                \
    GST_TYPE_CURLENDPOINT,                  \
    GstcurlendpointPrivate                   \
  )                                       \
)

typedef struct _GstcurlendpointElement
{
  KmsElementPadType type;
  gchar *description;
  GstElement *sink;
} GstcurlendpointElement;

struct _GstcurlendpointPrivate
{
  gboolean video;
  gboolean audio;
  gboolean data;
  GstElement *videoappsink;
  GstElement *audioappsink;
  GstElement *dataappsink;

  GHashTable *sinks;            /* <name, GstcurlendpointElement> */
};

G_DEFINE_TYPE_WITH_CODE (Gstcurlendpoint, gst_curlendpoint,
    KMS_TYPE_URI_ENDPOINT,
    GST_DEBUG_CATEGORY_INIT (gst_curlendpoint_debug_category, PLUGIN_NAME,
        0, "debug category for kurento dummy plugin"));

/* Object properties */
enum
{
  PROP_0,
  PROP_DATA,
  PROP_AUDIO,
  PROP_VIDEO,
  N_PROPERTIES
};

#define DEFAULT_HTTP_ENDPOINT_START FALSE

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL, };

static GstcurlendpointElement *
create_curlendpoint_element (KmsElementPadType type, const gchar * description,
    GstElement * sink)
{
  GstcurlendpointElement *dummy_sink;

  dummy_sink = g_slice_new0 (GstcurlendpointElement);
  dummy_sink->type = type;
  dummy_sink->description = g_strdup (description);
  dummy_sink->sink = g_object_ref (sink);

  return dummy_sink;
}

static void
destroy_curlendpoint_element (GstcurlendpointElement * sink)
{
  g_free (sink->description);
  g_clear_object (&sink->sink);

  g_slice_free (GstcurlendpointElement, sink);
}

static gboolean
set_audio_caps(GstElement * element, GstPad * pad) {
  GstCaps *caps;
  int caps_set;

  return TRUE;
  
  caps = gst_caps_new_simple ("audio/x-raw",
                              "format", G_TYPE_STRING, "S16LE",
                              "rate", G_TYPE_INT, 16000,
                              "channels", G_TYPE_INT, 1, NULL);
  caps_set = gst_pad_set_caps (pad, caps);
  gst_caps_unref (caps);
  if (!caps_set) {
    GST_ELEMENT_ERROR (element, CORE, NEGOTIATION, (NULL),
                       ("Can not set audio caps on sinkpad"));
    return FALSE;
  }
  return TRUE;
}

static void
set_location (Gstcurlendpoint * self, GstElement * element) {
  g_object_set(element, "location", KMS_URI_ENDPOINT(self)->uri, NULL);
  GST_INFO_OBJECT (element, "Streaming to %s", KMS_URI_ENDPOINT(self)->uri);
}

static void
kms_filter_element_connect_filter (Gstcurlendpoint * self,
    KmsElementPadType type, GstElement * filter)
{
  GstElement *queue = gst_element_factory_make ("queue", NULL);
  GstPad *target = gst_element_get_static_pad (queue, "sink");

  // g_object_set (queue, "leaky", 2, "max-size-buffers", 1000, NULL);

  gst_bin_add_many (GST_BIN (self), queue, filter, NULL);

  // self->priv->filter = filter;

  gst_element_link_many (queue, filter, NULL);
  gst_element_sync_state_with_parent (filter);
  gst_element_sync_state_with_parent (queue);

  kms_element_connect_sink_target (KMS_ELEMENT (self), target, type);
  g_object_unref (target);
}

static void
kms_filter_element_connect_passthrough (Gstcurlendpoint * self,
    KmsElementPadType type, GstElement * element)
{
  GstPad *target = gst_element_get_static_pad (element, "sink");

  kms_element_connect_sink_target (KMS_ELEMENT (self), target, type);
  g_object_unref (target);
}

static void
kms_filter_element_set_filter (Gstcurlendpoint * self, GstElement * filter)
{
  GstPad *sink = NULL;
  GstCaps *audio_caps = NULL, *video_caps = NULL;
  GstCaps *sink_caps = NULL;




  sink = gst_element_get_static_pad (filter, "sink");

  audio_caps = gst_caps_from_string (KMS_AGNOSTIC_AUDIO_CAPS);
  video_caps = gst_caps_from_string (KMS_AGNOSTIC_VIDEO_CAPS);

  sink_caps = gst_pad_query_caps (sink, NULL);


  //  KMS_FILTER_ELEMENT_LOCK (self);


  kms_filter_element_connect_filter (self, KMS_ELEMENT_PAD_TYPE_AUDIO, filter);
  kms_filter_element_connect_passthrough (self, KMS_ELEMENT_PAD_TYPE_VIDEO,
                                          kms_element_get_video_agnosticbin (KMS_ELEMENT (self)));
  /* Enable data pads */
  kms_filter_element_connect_passthrough (self, KMS_ELEMENT_PAD_TYPE_DATA,
      kms_element_get_data_tee (KMS_ELEMENT (self)));

  // KMS_FILTER_ELEMENT_UNLOCK (self);


  if (sink_caps != NULL)
    gst_caps_unref (sink_caps);


  if (audio_caps != NULL)
    gst_caps_unref (audio_caps);

  if (video_caps != NULL)
    gst_caps_unref (video_caps);

  if (sink != NULL)
    g_object_unref (sink);

}


static void
gst_curlendpoint_add_sinkpad (Gstcurlendpoint * self, KmsElementPadType type)
{
  GstElement **appsink;
  GstPad *sinkpad;
  gchar *name;

  switch (type) {
    case KMS_ELEMENT_PAD_TYPE_DATA:
      appsink = &self->priv->dataappsink;
      name = APPDATASINK;
      break;
    case KMS_ELEMENT_PAD_TYPE_AUDIO:
      appsink = &self->priv->audioappsink;
      name = APPAUDIOSINK;
      break;
    case KMS_ELEMENT_PAD_TYPE_VIDEO:
      appsink = &self->priv->videoappsink;
      name = APPVIDEOSINK;
      break;
    default:
      GST_ERROR_OBJECT (self, "Invalid pad type provided");
      return;
  }

  if (*appsink == NULL && type != KMS_ELEMENT_PAD_TYPE_AUDIO) {
    /* First time that appsink is created */
    *appsink = gst_element_factory_make ("fakesink", name);
    g_object_set (*appsink, "async", FALSE, "sync", FALSE, NULL);
    gst_bin_add (GST_BIN (self), *appsink);
    gst_element_sync_state_with_parent (*appsink);
  }
  if (*appsink == NULL && type == KMS_ELEMENT_PAD_TYPE_AUDIO) {
    /* First time that appsink is created */
    *appsink = gst_element_factory_make ("curlhttpsink", name);
    g_object_set (*appsink, "async", FALSE, "sync", FALSE, "max-lateness", -1, NULL);
    g_object_set (*appsink, "blocksize", 4096, "qos", FALSE,
                  "async", FALSE, "sync", FALSE, NULL);
    //g_object_set (*appsink, "async", FALSE, NULL);
    set_location (self, *appsink);
    kms_filter_element_set_filter(self, *appsink);
    return;
    /*    
    g_object_set (*appsink, "async", TRUE, "sync", TRUE, NULL);
    gst_bin_add (GST_BIN (self), *appsink);
    set_location (self, *appsink);
    gst_element_sync_state_with_parent (*appsink);
    */
  }

  sinkpad = gst_element_get_static_pad (*appsink, "sink");
  if (type == KMS_ELEMENT_PAD_TYPE_AUDIO) {
    set_audio_caps (*appsink, sinkpad);
      GST_ERROR_OBJECT (self, "Linking AUDIO PAD #######");
  }

  kms_element_connect_sink_target (KMS_ELEMENT (self), sinkpad, type);

  g_object_unref (sinkpad);
}

static void
gst_curlendpoint_set_property (GObject * object, guint property_id,
    const GValue * value, GParamSpec * pspec)
{
  Gstcurlendpoint *self = GST_CURLENDPOINT (object);
  gboolean val;

  KMS_ELEMENT_LOCK (KMS_ELEMENT (self));
  switch (property_id) {
    case PROP_DATA:
      val = g_value_get_boolean (value);
      if (val && !self->priv->data) {
        gst_curlendpoint_add_sinkpad (self, KMS_ELEMENT_PAD_TYPE_DATA);
      } else if (!val && self->priv->data) {
        kms_element_remove_sink_by_type (KMS_ELEMENT (self),
            KMS_ELEMENT_PAD_TYPE_DATA);
      } else {
        GST_DEBUG_OBJECT (self, "Operation without effect");
      }

      self->priv->data = val;
      break;
    case PROP_AUDIO:
      val = g_value_get_boolean (value);
      if (val && !self->priv->audio) {
        gst_curlendpoint_add_sinkpad (self, KMS_ELEMENT_PAD_TYPE_AUDIO);
      } else if (!val && self->priv->audio) {
        kms_element_remove_sink_by_type (KMS_ELEMENT (self),
            KMS_ELEMENT_PAD_TYPE_AUDIO);
      } else {
        GST_DEBUG_OBJECT (self, "Operation without effect");
      }

      self->priv->audio = val;
      break;
    case PROP_VIDEO:
      val = g_value_get_boolean (value);
      if (val && !self->priv->video) {
        gst_curlendpoint_add_sinkpad (self, KMS_ELEMENT_PAD_TYPE_VIDEO);
      } else if (!val && self->priv->video) {
        kms_element_remove_sink_by_type (KMS_ELEMENT (self),
            KMS_ELEMENT_PAD_TYPE_VIDEO);
      } else {
        GST_DEBUG_OBJECT (self, "Operation without effect");
      }

      self->priv->video = val;
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
  KMS_ELEMENT_UNLOCK (KMS_ELEMENT (self));
}

static void
gst_curlendpoint_get_property (GObject * object, guint property_id,
    GValue * value, GParamSpec * pspec)
{
  Gstcurlendpoint *self = GST_CURLENDPOINT (object);

  KMS_ELEMENT_LOCK (KMS_ELEMENT (self));
  switch (property_id) {
    case PROP_DATA:
      g_value_set_boolean (value, self->priv->data);
      break;
    case PROP_AUDIO:
      g_value_set_boolean (value, self->priv->audio);
      break;
    case PROP_VIDEO:
      g_value_set_boolean (value, self->priv->video);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
  KMS_ELEMENT_UNLOCK (KMS_ELEMENT (self));
}

static void
gst_curlendpoint_finalize (GObject * object)
{
  Gstcurlendpoint *self = GST_CURLENDPOINT (object);

  GST_DEBUG_OBJECT (self, "finalize");
  g_hash_table_unref (self->priv->sinks);

  /* chain up */
  G_OBJECT_CLASS (gst_curlendpoint_parent_class)->finalize (object);
}

static gboolean
gst_curlendpoint_request_new_sink_pad (KmsElement * obj, KmsElementPadType type,
    const gchar * description, const gchar * name)
{
  Gstcurlendpoint *self = GST_CURLENDPOINT (obj);
  GstcurlendpointElement *dummy;
  GstElement *sink;
  GstPad *sinkpad;

  GST_DEBUG_OBJECT (self, "Not implemented!!!");
  return FALSE;

  KMS_ELEMENT_LOCK (KMS_ELEMENT (self));

  if (g_hash_table_contains (self->priv->sinks, name)) {
    KMS_ELEMENT_UNLOCK (KMS_ELEMENT (self));

    return TRUE;
  }

  sink = gst_element_factory_make ("fakesink", NULL);
  g_object_set (sink, "async", FALSE, "sync", FALSE, NULL);

  dummy = create_curlendpoint_element (type, description, sink);

  if (!gst_bin_add (GST_BIN (self), sink)) {
    KMS_ELEMENT_UNLOCK (KMS_ELEMENT (self));
    destroy_curlendpoint_element (dummy);

    return FALSE;
  }

  g_hash_table_insert (self->priv->sinks, g_strdup (name), dummy);
  KMS_ELEMENT_UNLOCK (KMS_ELEMENT (self));

  gst_element_sync_state_with_parent (sink);

  sinkpad = gst_element_get_static_pad (sink, "sink");
  if (!set_audio_caps(sink, sinkpad)) {
    g_object_unref (sinkpad);
    return FALSE;
  }
  kms_element_connect_sink_target_full (KMS_ELEMENT (self), sinkpad, type,
      description, NULL, NULL);
  g_object_unref (sinkpad);

  return TRUE;
}
static gboolean
gst_curlendpoint_release_requested_sink_pad (KmsElement * obj, GstPad * pad)
{
  Gstcurlendpoint *self = GST_CURLENDPOINT (obj);
  GstcurlendpointElement *dummy;
  gchar *padname;

  padname = gst_pad_get_name (pad);

  KMS_ELEMENT_LOCK (KMS_ELEMENT (self));

  dummy = g_hash_table_lookup (self->priv->sinks, padname);

  if (dummy == NULL) {
    KMS_ELEMENT_UNLOCK (KMS_ELEMENT (self));

    return FALSE;
  }

  kms_element_remove_sink_by_type_full (obj, dummy->type, dummy->description);
  g_hash_table_remove (self->priv->sinks, padname);

  return TRUE;
}

static gboolean
gst_curlendpoint_started (KmsUriEndpoint * obj, GError ** error)
{
  Gstcurlendpoint * self = GST_CURLENDPOINT(obj);
  GstElement** audiosink = &self->priv->audioappsink;
  gst_element_sync_state_with_parent (*audiosink);
  // gst_element_set_state (*audiosink, GST_STATE_PLAYING);
  return TRUE;
}
static gboolean
gst_curlendpoint_stopped (KmsUriEndpoint * obj, GError ** error)
{
  return TRUE;
}

static gboolean
gst_curlendpoint_paused (KmsUriEndpoint * obj, GError ** error)
{
  return TRUE;
}
static void
gst_curlendpoint_class_init (GstcurlendpointClass * klass)
{
  KmsElementClass *kmselement_class;
  GstElementClass *gstelement_class;
  GObjectClass *gobject_class;
  KmsUriEndpointClass *urienpoint_class = KMS_URI_ENDPOINT_CLASS (klass);

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->set_property = gst_curlendpoint_set_property;
  gobject_class->get_property = gst_curlendpoint_get_property;
  gobject_class->finalize = gst_curlendpoint_finalize;

  kmselement_class = KMS_ELEMENT_CLASS (klass);
  kmselement_class->request_new_sink_pad =
      GST_DEBUG_FUNCPTR (gst_curlendpoint_request_new_sink_pad);
  kmselement_class->release_requested_sink_pad =
      GST_DEBUG_FUNCPTR (gst_curlendpoint_release_requested_sink_pad);

  urienpoint_class->stopped = gst_curlendpoint_stopped;
  urienpoint_class->started = gst_curlendpoint_started;
  urienpoint_class->paused = gst_curlendpoint_paused;

  gstelement_class = GST_ELEMENT_CLASS (klass);
  gst_element_class_set_details_simple (gstelement_class,
      "Gstcurlendpoint",
      "Generic",
      "Dummy sink element",
      "Santiago Carot-Nemesio <sancane.kurento@gmail.com>");

  obj_properties[PROP_DATA] = g_param_spec_boolean ("data",
      "Data", "Provides data on TRUE", FALSE,
      (G_PARAM_CONSTRUCT | G_PARAM_READWRITE));

  obj_properties[PROP_AUDIO] = g_param_spec_boolean ("audio",
      "Audio", "Provides audio on TRUE", FALSE,
      (G_PARAM_CONSTRUCT | G_PARAM_READWRITE));

  obj_properties[PROP_VIDEO] = g_param_spec_boolean ("video",
      "Video", "Provides video on TRUE", FALSE,
      (G_PARAM_CONSTRUCT | G_PARAM_READWRITE));

  g_object_class_install_properties (gobject_class,
      N_PROPERTIES, obj_properties);

  g_type_class_add_private (klass, sizeof (GstcurlendpointPrivate));
}

static void
gst_curlendpoint_init (Gstcurlendpoint * self)
{
  self->priv = GST_CURLENDPOINT_GET_PRIVATE (self);

  self->priv->sinks = g_hash_table_new_full (g_str_hash, g_str_equal, g_free,
      (GDestroyNotify) destroy_curlendpoint_element);
}

gboolean
gst_curlendpoint_plugin_init (GstPlugin * plugin)
{
  return gst_element_register (plugin, PLUGIN_NAME, GST_RANK_NONE,
      GST_TYPE_CURLENDPOINT);
}
