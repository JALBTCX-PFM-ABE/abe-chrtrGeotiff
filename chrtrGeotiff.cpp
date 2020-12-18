
/*********************************************************************************************

    This is public domain software that was developed by or for the U.S. Naval Oceanographic
    Office and/or the U.S. Army Corps of Engineers.

    This is a work of the U.S. Government. In accordance with 17 USC 105, copyright protection
    is not available for any work of the U.S. Government.

    Neither the United States Government, nor any employees of the United States Government,
    nor the author, makes any warranty, express or implied, without even the implied warranty
    of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE, or assumes any liability or
    responsibility for the accuracy, completeness, or usefulness of any information,
    apparatus, product, or process disclosed, or represents that its use would not infringe
    privately-owned rights. Reference herein to any specific commercial products, process,
    or service by trade name, trademark, manufacturer, or otherwise, does not necessarily
    constitute or imply its endorsement, recommendation, or favoring by the United States
    Government. The views and opinions of authors expressed herein do not necessarily state
    or reflect those of the United States Government, and shall not be used for advertising
    or product endorsement purposes.
*********************************************************************************************/


/****************************************  IMPORTANT NOTE  **********************************

    Comments in this file that start with / * ! or / / ! are being used by Doxygen to
    document the software.  Dashes in these comment blocks are used to create bullet lists.
    The lack of blank lines after a block of dash preceeded comments means that the next
    block of dash preceeded comments is a new, indented bullet list.  I've tried to keep the
    Doxygen formatting to a minimum but there are some other items (like <br> and <pre>)
    that need to be left alone.  If you see a comment that starts with / * ! or / / ! and
    there is something that looks a bit weird it is probably due to some arcane Doxygen
    syntax.  Be very careful modifying blocks of Doxygen comments.

*****************************************  IMPORTANT NOTE  **********************************/



#include "chrtrGeotiff.hpp"
#include "chrtrGeotiffHelp.hpp"


void set_defaults (OPTIONS *options);
void envin (OPTIONS *options);
void envout (OPTIONS *options);


chrtrGeotiff::chrtrGeotiff (int32_t *argc, char **argv, QWidget *parent)
  : QWizard (parent, 0)
{
  QResource::registerResource ("/icons.rcc");


  //  Set the main icon

  setWindowIcon (QIcon (":/icons/chrtrGeotiffWatermark.png"));


  //  Get the sample data for the color and sunshade examples.

  options.sample_pixmap = QPixmap (SAMPLE_WIDTH, SAMPLE_HEIGHT);
  uint8_t idata[2];
  QFile *dataFile = new QFile (":/icons/data.dat");
  options.sample_min = 99999.0;
  options.sample_max = -99999.0;

  if (dataFile->open (QIODevice::ReadOnly))
    {
      for (int32_t i = 0 ; i < SAMPLE_HEIGHT ; i++)
        {
          for (int32_t j = 0 ; j < SAMPLE_WIDTH ; j++)
            {
              dataFile->read ((char *) idata, 2);
              options.sample_data[i][j] = idata[1] * 256 + idata[0];

              options.sample_min = qMin ((float) options.sample_data[i][j], options.sample_min);
              options.sample_max = qMax ((float) options.sample_data[i][j], options.sample_max);
            }
        }
      dataFile->close ();
    }


  //  Set the normal defaults

  set_defaults (&options);


  //  Get the user's defaults if available

  envin (&options);


  // Set the application font

  QApplication::setFont (options.font);


  setWizardStyle (QWizard::ClassicStyle);


  setOption (HaveHelpButton, true);
  setOption (ExtendedWatermarkPixmap, false);

  connect (this, SIGNAL (helpRequested ()), this, SLOT (slotHelpClicked ()));


  area_file_name = tr ("NONE");


  //  Set the window size and location from the defaults

  this->resize (options.window_width, options.window_height);
  this->move (options.window_x, options.window_y);


  setPage (0, new startPage (argc, argv, this, &options));

  setPage (1, new surfacePage (this, &options));

  setPage (2, (ip = new imagePage (this, &options)));

  setPage (3, new runPage (this, &progress, &checkList));


  setButtonText (QWizard::CustomButton1, tr("&Run"));
  setOption (QWizard::HaveCustomButton1, true);
  button (QWizard::CustomButton1)->setToolTip (tr ("Start generating the GeoTIFF(s)"));
  button (QWizard::CustomButton1)->setWhatsThis (runText);
  connect (this, SIGNAL (customButtonClicked (int)), this, SLOT (slotCustomButtonClicked (int)));


  setStartId (0);
}


chrtrGeotiff::~chrtrGeotiff ()
{
}



void chrtrGeotiff::initializePage (int id)
{
  button (QWizard::HelpButton)->setIcon (QIcon (":/icons/contextHelp.png"));
  button (QWizard::CustomButton1)->setEnabled (false);


  switch (id)
    {
    case 0:
      break;

    case 1:
      break;

    case 2:
      options.transparent = field ("transparent_check").toBool ();
      options.caris = field ("caris_check").toBool ();
      options.grey = field ("grey_check").toBool ();
      options.dumb = field ("dumb_check").toBool ();
      options.elev = field ("elev_check").toBool ();
      options.cint = (float) field ("interval").toDouble ();

      if (options.grey)
        {
          ip->enable (NVFalse);
        }
      else
        {
          ip->enable (NVTrue);
        }
      break;

    case 3:
      button (QWizard::CustomButton1)->setEnabled (true);

      chrtr_file_name = field ("chrtr_file_edit").toString ();
      area_file_name = field ("area_file_edit").toString ();
      output_file_name = field ("output_file_edit").toString ();


      //  Save the output directory.  It might have been input manually instead of browsed.

      options.output_dir = QFileInfo (output_file_name).absoluteDir ().absolutePath ();

      options.azimuth = field ("sunAz").toDouble ();
      options.elevation = field ("sunEl").toDouble ();
      options.exaggeration = field ("sunEx").toDouble ();
      options.saturation = field ("satSpin").toDouble ();
      options.value = field ("valSpin").toDouble ();
      options.start_hsv = field ("startSpin").toDouble ();
      options.end_hsv = field ("endSpin").toDouble ();


      //  Use frame geometry to get the absolute x and y.

      QRect tmp = this->frameGeometry ();
      options.window_x = tmp.x ();
      options.window_y = tmp.y ();


      //  Use geometry to get the width and height.

      tmp = this->geometry ();
      options.window_width = tmp.width ();
      options.window_height = tmp.height ();


      //  Save the options.

      envout (&options);


      QString string;

      checkList->clear ();

      string = tr ("Input CHRTR file : ") + chrtr_file_name;
      checkList->addItem (string);

      string = tr ("Output file(s) : ") + output_file_name;
      checkList->addItem (string);

      if (!area_file_name.isEmpty ())
        {
          string = tr ("Area file : ") + area_file_name;
          checkList->addItem (string);
        }


      switch (options.transparent)
        {
        case false:
          string = tr ("Empty cells are blank");
          checkList->addItem (string);
          break;

        case true:
          string = tr ("Empty cells are transparent");
          checkList->addItem (string);
          break;
        }

      switch (options.caris)
        {
        case false:
          string = tr ("LZW compressed output format");
          checkList->addItem (string);
          break;

        case true:
          string = tr ("Brain-dead Caris output format");
          checkList->addItem (string);
          break;
        }

      switch (options.elev)
        {
        case false:
          string = tr ("Data will be output as depths");
          checkList->addItem (string);
          break;

        case true:
          string = tr ("Data will be output as elevations");
          checkList->addItem (string);
          break;
        }


      if (options.cint != 0.0)
        {
          contour = NVTrue;
          string = QString (tr ("ESRI contour file will be generated with a contour interval of %1")).arg (options.cint, 6, 'f', 2);
          checkList->addItem (string);
        }
      else
        {
          contour = NVFalse;
        }



      switch (options.units)
        {
        case 0:
          string = tr ("Units : Meters");
          checkList->addItem (string);
          break;

        case 1:
          if (options.dumb)
            {
              string = tr ("Units : Fathoms (1500 m/sec)");
            }
          else
            {
              string = tr ("Units : Fathoms (4800 ft/sec)");
            }
          checkList->addItem (string);
          break;
        }



      QListWidgetItem *cur;

      if (!options.grey)
        {
          switch (options.transparent)
            {
            case false:
              string = tr ("Empty cells are blank");
              checkList->addItem (string);
              break;

            case true:
              string = tr ("Empty cells are transparent");
              checkList->addItem (string);
              break;
            }


          switch (options.restart)
            {
            case false:
              string = tr ("Color map is continuous from minimum to maximum");
              checkList->addItem (string);
              break;

            case true:
              string = tr ("Color map starts over at zero boundary");
              checkList->addItem (string);
              break;
            }

          string = QString (tr ("Sun Azimuth : %1")).arg (options.azimuth, 6, 'f', 2);
          checkList->addItem (string);


          string = QString (tr ("Sun Elevation : %1")).arg (options.elevation, 5, 'f', 2);
          checkList->addItem (string);


          string = QString (tr ("Vertical Exaggeration : %1")).arg (options.exaggeration, 4, 'f', 2);
          checkList->addItem (string);


          string = QString (tr ("Color Saturation : %1")).arg (options.saturation, 4, 'f', 2);
          checkList->addItem (string);


          string = QString (tr ("Color Value : %1")).arg (options.value, 4, 'f', 2);
          checkList->addItem (string);


          string = QString (tr ("Start Hue Value : %1")).arg (options.start_hsv, 6, 'f', 2);
          checkList->addItem (string);


          string = QString (tr ("End Hue Value : %1")).arg (options.end_hsv, 6, 'f', 2);
          cur = new QListWidgetItem (string);
        }
      else
        {
          string = QString (tr ("Output is 32 bit floating point elevations"));
          cur = new QListWidgetItem (string);
        }


      checkList->addItem (cur);
      checkList->setCurrentItem (cur);
      checkList->scrollToItem (cur);


      if (contour)
        {
          progress.cbox->show ();
        }
      else
        {
          progress.cbox->hide ();
        }


      break;
    }
}



void chrtrGeotiff::cleanupPage (int id)
{
  switch (id)
    {
    case 0:
      break;

    case 1:
      break;

    case 2:
      break;

    case 3:
      break;
    }
}



void chrtrGeotiff::slotHelpClicked ()
{
  QWhatsThis::enterWhatsThisMode ();
}



//  This is where the fun stuff happens.

void 
chrtrGeotiff::slotCustomButtonClicked (int id __attribute__ ((unused)))
{
  int32_t             chrtr_handle, width, height, x_start, y_start, count = 0, header_width, header_height, c_index;
  float               *ar, *current_row, *next_row, min_z, max_z, range[2] = {0.0, 0.0}, shade_factor, null_value = 0.0;
  double              conversion_factor, mid_y_radians, x_cell_size, y_cell_size, x_cell_degrees, y_cell_degrees;
  double              polygon_x[200], polygon_y[200];
  NV_F64_XYMBR        mbr;
  NV_F64_MBR          header_mbr;
  uint8_t             cross_zero = NVFalse;
  CHRTR_HEADER        chrtr_header;
  CHRTR2_HEADER       chrtr2_header;
  CHRTR2_RECORD       *chrtr2_record = NULL;
  char                basename[512], name[512], chrtr_name[512], area_file[512];
  QString             string;


  int32_t scribe (int32_t, int32_t, float, float, float, float, float *, char *, OPTIONS *, double, double);


  QApplication::setOverrideCursor (Qt::WaitCursor);


  button (QWizard::FinishButton)->setEnabled (false);
  button (QWizard::BackButton)->setEnabled (false);
  button (QWizard::CustomButton1)->setEnabled (false);


  //  Note - The sunopts and the color_array get set in display_sample_data (imagePage.cpp).  No point in
  //  doing it twice.

  strcpy (chrtr_name, chrtr_file_name.toLatin1 ());


  x_start = 0;
  y_start = 0;


  if (options.chrtr2)
    {
      if ((chrtr_handle = chrtr2_open_file (chrtr_name, &chrtr2_header, CHRTR2_READONLY)) < 0)
        {
          chrtr2_perror ();
          exit (-1);
        }


      header_width = width = chrtr2_header.width;
      header_height = height = chrtr2_header.height;


      header_mbr.wlon = mbr.min_x = chrtr2_header.mbr.wlon;
      header_mbr.elon = mbr.max_x = chrtr2_header.mbr.elon;
      header_mbr.slat = mbr.min_y = chrtr2_header.mbr.slat;
      header_mbr.nlat = mbr.max_y = chrtr2_header.mbr.nlat;

      y_cell_degrees = chrtr2_header.lat_grid_size_degrees;
      x_cell_degrees = chrtr2_header.lon_grid_size_degrees;

      min_z = CHRTR2_NULL_Z_VALUE;
      max_z = -CHRTR2_NULL_Z_VALUE;
      null_value = CHRTR2_NULL_Z_VALUE;
    }
  else
    {
      if ((chrtr_handle = open_chrtr (chrtr_name, &chrtr_header)) < 0)
        {
          perror (chrtr_name);
          exit (-1);
        }


      header_width = width = chrtr_header.width;
      header_height = height = chrtr_header.height;


      header_mbr.wlon = mbr.min_x = chrtr_header.wlon;
      header_mbr.elon = mbr.max_x = chrtr_header.elon;
      header_mbr.slat = mbr.min_y = chrtr_header.slat;
      header_mbr.nlat = mbr.max_y = chrtr_header.nlat;

      y_cell_degrees = x_cell_degrees = chrtr_header.grid_minutes / 60.0;

      min_z = CHRTRNULL;
      max_z = -CHRTRNULL;
      null_value = CHRTRNULL;
    }


  strcpy (basename, output_file_name.toLatin1 ());

  if (strcmp (&basename[strlen (basename) - 4], ".tif")) strcat (basename, ".tif");

  strcpy (name, basename);


  //  Check for an area file.

  if (!area_file_name.isEmpty ())
    {
      strcpy (area_file, area_file_name.toLatin1 ());

      if (!get_area_mbr (area_file, &count, polygon_x, polygon_y, &mbr))
        {
          QString qstring = QString (tr ("Error reading area file %1\nReason : %2")).arg (area_file_name).arg (QString (strerror (errno)));
          QMessageBox::critical (this, tr ("chrtrGeotiff"), qstring);
          exit (-1);
        }


      if (mbr.min_y > header_mbr.nlat || mbr.max_y < header_mbr.slat || mbr.min_x > header_mbr.elon || mbr.max_x < header_mbr.wlon)
        {
          QString qstring = QString (tr ("Specified area is completely outside of the CHRTR bounds!"));
          QMessageBox::critical (this, tr ("chrtrGeotiff"), qstring);
          exit (-1);
        }


      //  Match to nearest cell

      x_start = NINT ((mbr.min_x - header_mbr.wlon) / x_cell_degrees);
      y_start = NINT ((mbr.min_y - header_mbr.slat) / y_cell_degrees);
      width = NINT ((mbr.max_x - mbr.min_x) / x_cell_degrees);
      height = NINT ((mbr.max_y - mbr.min_y) / y_cell_degrees);


      //  Adjust to CHRTR bounds if necessary

      if (x_start < 0) x_start = 0;
      if (y_start < 0) y_start = 0;
      if (x_start + width > header_width) width = header_width - x_start;
      if (y_start + height > header_height) height = header_height - y_start;


      //  Redefine bounds

      mbr.min_x = header_mbr.wlon + x_start * x_cell_degrees;
      mbr.min_y = header_mbr.slat + y_start * y_cell_degrees;
      mbr.max_x = mbr.min_x + width * x_cell_degrees;
      mbr.max_y = mbr.min_y + height * y_cell_degrees;
    }


  progress.mbar->setRange (0, height);


  //  Compute cell sizes for sunshading.

  mid_y_radians = (header_mbr.nlat - header_mbr.slat) * 0.0174532925199432957692;
  conversion_factor = cos (mid_y_radians);
  x_cell_size = x_cell_degrees * 111120.0 * conversion_factor;
  y_cell_size = y_cell_degrees * 111120.0;


  uint8_t *red = NULL, *blue = NULL, *green = NULL, *alpha = NULL;

  if ((red = (uint8_t *) calloc (width, sizeof (uint8_t))) == NULL)
    {
      perror ("Allocating red in chrtrGeotiff.cpp");
      exit (-1);
    }
  if ((green = (uint8_t *) calloc (width, sizeof (uint8_t))) == NULL)
    {
      perror ("Allocating green in chrtrGeotiff.cpp");
      exit (-1);
    }
  if ((blue = (uint8_t *) calloc (width, sizeof (uint8_t))) == NULL)
    {
      perror ("Allocating blue in chrtrGeotiff.cpp");
      exit (-1);
    }
  if ((alpha = (uint8_t *) calloc (width, sizeof (uint8_t))) == NULL)
    {
      perror ("Allocating alpha in chrtrGeotiff.cpp");
      exit (-1);
    }
  if ((next_row = (float *) calloc (width, sizeof (float))) == NULL)
    {
      perror ("Allocating next_row in chrtrGeotiff.cpp");
      exit (-1);
    }
  if ((current_row = (float *) calloc (width, sizeof (float))) == NULL)
    {
      perror ("Allocating current_row in chrtrGeotiff.cpp");
      exit (-1);
    }


  if (options.chrtr2)
    {
      if ((chrtr2_record = (CHRTR2_RECORD *) calloc (width, sizeof (CHRTR2_RECORD))) == NULL)
        {
          perror ("Allocating chrtr2_record in chrtrGeotiff.cpp");
          exit (-1);
        }
    }


  int32_t ar_size = width * height;

  ar = (float *) calloc (ar_size, sizeof (float));
  if (ar == NULL)
    {
      perror ("Allocating ar in chrtrGeotiff.cpp");
      exit (-1);
    }


  //  Scan for min/max and load the grid array.

  for (int32_t i = 0, m = 1 ; i < height ; i++, m++)
    {
      if (options.chrtr2)
        {
          chrtr2_read_row (chrtr_handle, y_start + i, x_start, width, chrtr2_record);

          for (int32_t j = 0 ; j < width ; j++)
            {
              float z_value = chrtr2_record[j].z;

              if (chrtr2_record[j].status)
                {
                  if (options.units)
                    {
                      if (options.dumb)
                        {
                          z_value /= 1.875;
                        }
                      else
                        {
                          z_value /= 1.8288;
                        }
                    }


                  if (options.elev) z_value = -z_value;


                  //  Load the 1D array.

                  ar[i * width + j] = z_value; 


                  //  Compute the min/max

                  min_z = qMin (min_z, z_value);
                  max_z = qMax (max_z, z_value);
                }
              else
                {
                  //  Load the 1D array.

                  ar[i * width + j] = null_value;
                }
            }
        }
      else
        {
          read_chrtr (chrtr_handle, y_start + i, x_start, width, current_row);


          for (int32_t j = 0 ; j < width ; j++)
            {
              if (current_row[j] < null_value)
                {
                  float z_value = current_row[j];

                  if (options.units)
                    {
                      if (options.dumb)
                        {
                          z_value /= 1.875;
                        }
                      else
                        {
                          z_value /= 1.8288;
                        }
                    }


                  if (options.elev) z_value = -z_value;


                  //  Load the 1D array.

                  ar[i * width + j] = z_value; 


                  //  Compute the min/max

                  min_z = qMin (min_z, z_value);
                  max_z = qMax (max_z, z_value);
                }
              else
                {
                  //  Load the 1D array.

                  ar[i * width + j] = null_value; 
                }
            }
        }


      progress.mbar->setValue (m);

      qApp->processEvents ();
    }


  progress.mbar->setValue (height);


  if (options.chrtr2) free (chrtr2_record);


  progress.gbar->setRange (0, height);


  if (options.restart && min_z < 0.0)
    {
      range[0] = -min_z;
      range[1] = max_z;

      cross_zero = NVTrue;
    }
  else
    {
      range[0] = max_z - min_z;

      cross_zero = NVFalse;
    }


  GDALDataset         *df;
  char                *wkt = NULL;
  GDALRasterBand      *bd[4];
  double              trans[6];
  GDALDriver          *gt;
  char                **papszOptions = NULL;


  //  Set up the output GeoTIFF file.

  GDALAllRegister ();

  gt = GetGDALDriverManager ()->GetDriverByName ("GTiff");
  if (!gt)
    {
      fprintf (stderr, "Could not get GTiff driver\n");
      exit (-1);
    }

  int32_t bands = 3;
  if (options.transparent) bands = 4;


  //  Stupid Caris software can't read normal files!

  if (options.caris)
    {
      papszOptions = CSLSetNameValue (papszOptions, "COMPRESS", "PACKBITS");
    }
  else
    {
      papszOptions = CSLSetNameValue (papszOptions, "TILED", "NO");
      papszOptions = CSLSetNameValue (papszOptions, "COMPRESS", "LZW");
    }

  if (options.grey)
    {
      bands = 1;
      df = gt->Create (name, width, height, bands, GDT_Float32, papszOptions);
    }
  else
    {
      df = gt->Create (name, width, height, bands, GDT_Byte, papszOptions);
    }

  if (df == NULL)
    {
      fprintf (stderr, "Could not create %s\n", name);
      exit (-1);
    }

  trans[0] = mbr.min_x;
  trans[1] = x_cell_degrees;
  trans[2] = 0.0;
  trans[3] = mbr.max_y;
  trans[4] = 0.0;
  trans[5] = -y_cell_degrees;
  df->SetGeoTransform (trans);

  char wkt_str[1024];
  strcpy (wkt_str, "COMPD_CS[\"WGS84 with WGS84E Z\",GEOGCS[\"WGS 84\",DATUM[\"WGS_1984\",SPHEROID[\"WGS 84\",6378137,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]],TOWGS84[0,0,0,0,0,0,0],AUTHORITY[\"EPSG\",\"6326\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"degree\",0.01745329251994328,AUTHORITY[\"EPSG\",\"9108\"]],AXIS[\"Lat\",NORTH],AXIS[\"Long\",EAST],AUTHORITY[\"EPSG\",\"4326\"]],VERT_CS[\"ellipsoid Z in meters\",VERT_DATUM[\"Ellipsoid\",2002],UNIT[\"metre\",1],AXIS[\"Z\",UP]]]");
  wkt = wkt_str;

  df->SetProjection (wkt);


  for (int32_t i = 0 ; i < bands ; i++) bd[i] = df->GetRasterBand (i + 1);


  if (options.grey) bd[0]->SetNoDataValue (null_value);


  for (int32_t i = height - 1, k = 0 ; i >= 0 ; i--, k++)
    {
      CPLErr err;

      if (options.grey)
        {
          for (int32_t j = 0 ; j < width ; j++) current_row[j] = ar[(y_start + i) * width + j];
        }
      else
        {
          if (i == (height - 1))
            {
              for (int32_t j = 0 ; j < width ; j++) current_row[j] = ar[(y_start + i) * width + j];
              memcpy (next_row, current_row, width * sizeof (float));
            }
          else
            {
              memcpy (current_row, next_row, width * sizeof (float));
              for (int32_t j = 0 ; j < width ; j++) next_row[j] = ar[(y_start + i) * width + j];
            }
        }


      if (options.grey)
        {
          err = bd[0]->RasterIO (GF_Write, 0, k, width, 1, current_row, width, 1, GDT_Float32, 0, 0);
        }
      else
        {
          for (int32_t j = 0 ; j < width ; j++)
            {
              if (cross_zero)
                {
                  if (current_row[j] < 0.0)
                    {
                      c_index = (int32_t) (NUMHUES - (int32_t) (fabsf ((current_row[j] - min_z) / range[0] * NUMHUES))) * NUMSHADES;
                    }
                  else
                    {
                      c_index = (int32_t) (NUMHUES - (int32_t) (fabsf (current_row[j]) / range[1] * NUMHUES)) * NUMSHADES;
                    }
                }
              else
                {
                  c_index = (int32_t) (NUMHUES - (int32_t) (fabsf ((current_row[j] - min_z) / range[0] * NUMHUES))) * NUMSHADES;
                }

              if (current_row[j] >= null_value) c_index = -2; 

              shade_factor = sunshade (next_row, current_row, j, &options.sunopts, x_cell_size, y_cell_size);

              if (shade_factor < 0.0) shade_factor = options.sunopts.min_shade;

              c_index -= NINT (NUMSHADES * shade_factor + 0.5);


              if (c_index >= 0)
                {
                  red[j] = options.color_array[c_index].red ();
                  green[j] = options.color_array[c_index].green ();
                  blue[j] = options.color_array[c_index].blue ();
                  alpha[j] = 255;
                }
              else
                {
                  red[j] = green[j] = blue[j] = alpha[j] = 0;
                }
            }

          err = bd[0]->RasterIO (GF_Write, 0, k, width, 1, red, width, 1, GDT_Byte, 0, 0);
          err = bd[1]->RasterIO (GF_Write, 0, k, width, 1, green, width, 1, GDT_Byte, 0, 0);
          err = bd[2]->RasterIO (GF_Write, 0, k, width, 1, blue, width, 1, GDT_Byte, 0, 0);
          if (options.transparent) err = bd[3]->RasterIO (GF_Write, 0, k, width, 1, alpha, width, 1, GDT_Byte, 0, 0);
        }

      if (err == CE_Failure)
        {
          checkList->clear ();

          string = QString (tr ("Failed a TIFF scanline write - row %1")).arg (i);
          checkList->addItem (string);
        }


      progress.gbar->setValue (k + 1);

      qApp->processEvents ();
    }


  checkList->clear ();


  string = QString (tr ("Created TIFF file %1")).arg (name);
  checkList->addItem (string);

  string = QString (tr ("%1 rows by %2 columns")).arg (height).arg (width);
  checkList->addItem (string);


  if (options.chrtr2)
    {
      chrtr2_close_file (chrtr_handle);
    }
  else
    {
      close_chrtr (chrtr_handle);
    }


  free (red);
  free (green);
  free (blue);
  free (alpha);
  free (next_row);
  free (current_row);


  delete df;


  //  Contouring if requested.

  progress.cbar->setRange (0, 0);

  if (contour)
    {
      int32_t num_contours = scribe (width, height, mbr.min_x, mbr.min_y, min_z, max_z, ar, name, &options, x_cell_degrees, y_cell_degrees);


      string.sprintf (tr ("Generated %d contour segments").toLatin1 (), num_contours);
      QListWidgetItem *cur = new QListWidgetItem (string);


      checkList->addItem (cur);
      checkList->setCurrentItem (cur);
      checkList->scrollToItem (cur);
    }


  free (ar);


  progress.cbar->setRange (0, 100);
  progress.cbar->setValue (100);
  qApp->processEvents ();


  button (QWizard::FinishButton)->setEnabled (true);
  button (QWizard::CancelButton)->setEnabled (false);


  QApplication::restoreOverrideCursor ();


  checkList->addItem (" ");
  QListWidgetItem *cur = new QListWidgetItem (tr ("Conversion complete, press Finish to exit."));

  checkList->addItem (cur);
  checkList->setCurrentItem (cur);
  checkList->scrollToItem (cur);
}
