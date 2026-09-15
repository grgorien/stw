#include <gtk/gtk.h>
#include <stdio.h>

typedef struct {
  GtkWidget *label;
  GtkWidget *progress_bar;
  guint timer_id;
  gint64 start_time;
  gint64 elapsed_time;
  gboolean is_running;
} Base60;

static gint64 get_total_elapsed_ms(Base60 *app) {
  gint64 total = app->elapsed_time;
  if (app->is_running) {
    total += g_get_monotonic_time() - app->start_time;
  }
  return total;
}

static void update_label_text(Base60 *app) {
  gint64 total_ms = get_total_elapsed_ms(app) / 1000;
  int hours = (total_ms / 3600000);
  int minutes = (total_ms / 60000) % 60;
  int seconds = (total_ms / 1000) % 60;
  int centiseconds = (total_ms / 10) % 100;

  char time_string[32];
  char markup_buffer[128];

  snprintf(time_string, sizeof(time_string), "%02d:%02d:%02d.%02d", hours,
           minutes, seconds, centiseconds);
  snprintf(markup_buffer, sizeof(markup_buffer),
           "<span font='28' weight='bold'>%s</span>", time_string);
  gtk_label_set_markup(GTK_LABEL(app->label), markup_buffer);
}

static void draw_progress(GtkDrawingArea *area, cairo_t *cr, int width,
                          int height, gpointer user_data) {
  (void)area; // compiler supression
  Base60 *app = (Base60 *)user_data;
  gint64 total_ms = get_total_elapsed_ms(app) / 1000;
  gint64 ms_in_minute = total_ms % 60000;
  double fraction = (double)ms_in_minute / 60000.0; // i.e 0.0 to 0.1
  cairo_set_source_rgb(cr, 0.12, 0.12, 0.14);       // #0c0c0e
  cairo_rectangle(cr, 0, 0, width, height);
  cairo_fill(cr);

  cairo_set_source_rgb(cr, 0.18, 0.58, 0.82); // #123952
  cairo_rectangle(cr, 0, 0, (int)(width * fraction), height);
  cairo_fill(cr);
}

static gboolean on_timeout_tick(gpointer user_data) {
  Base60 *app = (Base60 *)user_data;
  if (!app->is_running) {
    return G_SOURCE_REMOVE;
  }
  update_label_text(app);
  gtk_widget_queue_draw(app->progress_bar);
  return G_SOURCE_CONTINUE;
}

static void stopwatch_stop(Base60 *app) {
  if (app->timer_id > 0) {
    g_source_remove(app->timer_id);
    app->timer_id = 0;
  }
  app->elapsed_time += g_get_monotonic_time() - app->start_time;
  app->is_running = FALSE;
}

static gboolean stopwatch_start(Base60 *app) {
  if (app->is_running) {
    return FALSE;
  }
  app->start_time = g_get_monotonic_time();
  app->timer_id = g_timeout_add(10, (GSourceFunc)on_timeout_tick, app);
  if (app->timer_id == 0) {
    return FALSE;
  }
  app->is_running = TRUE;
  return TRUE;
}

static void stopwatch_toggle(Base60 *app) {
  if (app->is_running) {
    stopwatch_stop(app);
  } else {
    stopwatch_start(app);
  }
  update_label_text(app);
  gtk_widget_queue_draw(app->progress_bar);
}

static void stopwatch_reset(Base60 *app) {
  stopwatch_stop(app);
  app->elapsed_time = 0;
  app->is_running = FALSE;
  update_label_text(app);
  gtk_widget_queue_draw(app->progress_bar);
}

// G_GNUC_UNUSED clear compiler warning
static gboolean on_key_pressed(G_GNUC_UNUSED GtkEventControllerKey *controller,
                               guint keyval, G_GNUC_UNUSED guint keycode,
                               GdkModifierType state, gpointer user_data) {
  Base60 *app = (Base60 *)user_data;

  if (keyval == GDK_KEY_space) {
    stopwatch_toggle(app);
    return TRUE;
  }

  if ((keyval == GDK_KEY_C || keyval == GDK_KEY_c) &&
      (state & GDK_SHIFT_MASK)) {
    stopwatch_reset(app);
    return TRUE;
  }

  return FALSE;
}

static void load_css(void) {
  GtkCssProvider *provider = gtk_css_provider_new();
  gtk_css_provider_load_from_string(provider, "window {"
                                              " background-color: #1a1a1e;"
                                              "}"
                                              "label {"
                                              " color: #e8e8ec;"
                                              "}"
                                              "#help-label {"
                                              " color: #5a5a6a;"
                                              " font-size: 12px;"
                                              "}");

  gtk_style_context_add_provider_for_display(
      gdk_display_get_default(), GTK_STYLE_PROVIDER(provider),
      GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
  g_object_unref(provider);
}

static void on_activate(GtkApplication *gapp, gpointer user_data) {
  Base60 *app = (Base60 *)user_data;
  load_css();

  GtkWidget *window = gtk_application_window_new(gapp);
  gtk_window_set_title(GTK_WINDOW(window), "stw");
  gtk_window_set_default_size(GTK_WINDOW(window), 250, 120);
  gtk_window_set_resizable(GTK_WINDOW(window), FALSE);

  GtkWidget *outer_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

  app->progress_bar = gtk_drawing_area_new();
  gtk_widget_set_size_request(app->progress_bar, -1, 5);
  gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(app->progress_bar),
                                 draw_progress, app, NULL);
  gtk_box_append(GTK_BOX(outer_box), app->progress_bar);

  GtkWidget *inner_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
  gtk_widget_set_halign(inner_box, GTK_ALIGN_CENTER);
  gtk_widget_set_valign(inner_box, GTK_ALIGN_CENTER);
  gtk_widget_set_vexpand(inner_box, TRUE);

  app->label = gtk_label_new(NULL);
  gtk_label_set_markup(GTK_LABEL(app->label),
                       "<span font='28' weight='bold'>00:00:00.00</span>");
  gtk_box_append(GTK_BOX(inner_box), app->label);

  GtkWidget *help_label = gtk_label_new("[Space] Start/Stop | [Shift+C] Reset");
  gtk_widget_set_name(help_label, "help_label");
  gtk_box_append(GTK_BOX(inner_box), help_label);

  gtk_box_append(GTK_BOX(outer_box), inner_box);
  gtk_window_set_child(GTK_WINDOW(window), outer_box);

  GtkEventController *controller = gtk_event_controller_key_new();
  g_signal_connect(controller, "key-pressed", G_CALLBACK(on_key_pressed), app);
  gtk_widget_add_controller(window, controller);

  gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char **argv) {
  if (argc > 1 && strcmp(argv[1], "--version") == 0) {
    printf("stw %s\n", APP_VERSION);
    return 0;
  }
  // change name to general, make kanam job
  Base60 app_data = {.label = NULL,
                     .progress_bar = NULL,
                     .timer_id = 0,
                     .start_time = 0,
                     .elapsed_time = 0,
                     .is_running = FALSE};

  GtkApplication *app =
      gtk_application_new("com.stw.stopwatch", G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect(app, "activate", G_CALLBACK(on_activate), &app_data);

  int status = g_application_run(G_APPLICATION(app), argc, argv);
  g_object_unref(app);
  return status;
}
