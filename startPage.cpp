
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



#include "startPage.hpp"
#include "startPageHelp.hpp"

startPage::startPage (int32_t *argc, char **argv, QWidget *parent, OPTIONS *op):
  QWizardPage (parent)
{
  options = op;


  setPixmap (QWizard::WatermarkPixmap, QPixmap(":/icons/chrtrGeotiffWatermark.png"));


  setTitle (tr ("Introduction"));

  setWhatsThis (tr ("See, it really works!"));

  QLabel *label = new QLabel (tr ("chrtrGeotiff is a tool for creating a GeoTIFF file of a surface stored "
				  "in a CHRTR file.  Help is available "
                                  "by clicking on the Help button and then clicking on the item for which "
                                  "you want help.  Select a CHRTR file below.  You may then change the default "
                                  "output file name and, optionally, select an area file to limit the extent "
                                  "of the GeoTIFF file that is created.  Click <b>Next</b> to continue or "
                                  "<b>Cancel</b> to exit."));
  label->setWordWrap (true);


  QVBoxLayout *vbox = new QVBoxLayout (this);
  vbox->addWidget (label);
  vbox->addStretch (10);


  QHBoxLayout *chrtr_file_box = new QHBoxLayout (0);
  chrtr_file_box->setSpacing (8);

  vbox->addLayout (chrtr_file_box);


  QLabel *chrtr_file_label = new QLabel (tr ("CHRTR File"), this);
  chrtr_file_box->addWidget (chrtr_file_label, 1);

  chrtr_file_edit = new QLineEdit (this);
  chrtr_file_edit->setReadOnly (true);
  chrtr_file_box->addWidget (chrtr_file_edit, 10);

  QPushButton *chrtr_file_browse = new QPushButton (tr ("Browse..."), this);
  chrtr_file_box->addWidget (chrtr_file_browse, 1);

  chrtr_file_label->setWhatsThis (chrtr_fileText);
  chrtr_file_edit->setWhatsThis (chrtr_fileText);
  chrtr_file_browse->setWhatsThis (chrtr_fileBrowseText);

  connect (chrtr_file_browse, SIGNAL (clicked ()), this, SLOT (slotCHRTRFileBrowse ()));



  QHBoxLayout *output_box = new QHBoxLayout (0);
  output_box->setSpacing (8);

  vbox->addLayout (output_box);


  QLabel *output_file_label = new QLabel (tr ("Output GeoTIFF File"), this);
  output_box->addWidget (output_file_label, 1);

  output_file_edit = new QLineEdit (this);
  output_box->addWidget (output_file_edit, 10);

  QPushButton *output_file_browse = new QPushButton (tr ("Browse..."), this);
  output_box->addWidget (output_file_browse, 1);

  output_file_label->setWhatsThis (output_fileText);
  output_file_edit->setWhatsThis (output_fileText);
  output_file_browse->setWhatsThis (output_fileBrowseText);

  connect (output_file_browse, SIGNAL (clicked ()), this, SLOT (slotOutputFileBrowse ()));


  QHBoxLayout *area_box = new QHBoxLayout (0);
  area_box->setSpacing (8);

  vbox->addLayout (area_box);


  QLabel *area_file_label = new QLabel (tr ("Optional Area File"), this);
  area_box->addWidget (area_file_label, 1);

  area_file_edit = new QLineEdit (this);
  area_file_edit->setReadOnly (true);
  area_box->addWidget (area_file_edit, 10);

  QPushButton *area_file_browse = new QPushButton (tr ("Browse..."), this);
  area_box->addWidget (area_file_browse, 1);


  area_file_label->setWhatsThis (area_fileText);
  area_file_edit->setWhatsThis (area_fileText);
  area_file_browse->setWhatsThis (area_fileBrowseText);

  connect (area_file_browse, SIGNAL (clicked ()), this, SLOT (slotAreaFileBrowse ()));


  if (*argc == 2)
    {
      if (strstr (argv[1], ".ch2"))
        {
          CHRTR2_HEADER chrtr2_header;
          int32_t chrtr_handle = -1;
          char chrtr_name[512];

          QString chrtr_file_name = QString (argv[1]);

          strcpy (chrtr_name, argv[1]);

          chrtr_handle = chrtr2_open_file (chrtr_name, &chrtr2_header, CHRTR2_READONLY);

          if (chrtr_handle >= 0)
            {
              chrtr_file_edit->setText (chrtr_file_name);


              //  If one hasn't been set, set the output TIFF filename.

              if (output_file_edit->text ().isEmpty ())
                { 
                  QString output_file_name = chrtr_file_name + ".tif";
                  output_file_edit->setText (output_file_name);
                }

              chrtr2_close_file (chrtr_handle);
            }

          options->chrtr2 = NVTrue;
        }
      else
        {
          CHRTR_HEADER chrtr_header;
          int32_t chrtr_handle = -1;
          char chrtr_name[512];

          QString chrtr_file_name = QString (argv[1]);

          strcpy (chrtr_name, argv[1]);

          chrtr_handle = open_chrtr (chrtr_name, &chrtr_header);

          if (chrtr_handle >= 0)
            {
              chrtr_file_edit->setText (chrtr_file_name);


              //  If one hasn't been set, set the output TIFF filename.

              if (output_file_edit->text ().isEmpty ())
                { 
                  QString output_file_name = chrtr_file_name + ".tif";
                  output_file_edit->setText (output_file_name);
                }

              close_chrtr (chrtr_handle);
            }

          options->chrtr2 = NVTrue;
        }
    }


  if (!chrtr_file_edit->text ().isEmpty ())
    {
      registerField ("chrtr_file_edit", chrtr_file_edit);
    }
  else
    {
      registerField ("chrtr_file_edit*", chrtr_file_edit);
    }


  registerField ("output_file_edit", output_file_edit);
  registerField ("area_file_edit", area_file_edit);
}



void startPage::slotCHRTRFileBrowse ()
{
  CHRTR_HEADER        chrtr_header;
  CHRTR2_HEADER       chrtr2_header;
  QStringList         files, filters;
  QString             file;
  int32_t             chrtr_handle = -1;
  char                chrtr_name[512];


  QFileDialog *fd = new QFileDialog (this, tr ("chrtrGeotiff Open CHRTR File"));
  fd->setViewMode (QFileDialog::List);


  //  Always add the current working directory and the last used directory to the sidebar URLs in case we're running from the command line.
  //  This function is in the nvutility library.

  setSidebarUrls (fd, options->input_dir);


  filters << tr ("CHRTR2 (*.ch2)")
          << tr ("CHRTR (*.fin)");

  fd->setNameFilters (filters);
  fd->setFileMode (QFileDialog::ExistingFile);
  fd->selectNameFilter (tr ("CHRTR2 (*.ch2)"));

  if (fd->exec () == QDialog::Accepted)
    {
      files = fd->selectedFiles ();

      QString chrtr_file_name = files.at (0);


      if (!chrtr_file_name.isEmpty())
        {
          strcpy (chrtr_name, chrtr_file_name.toLatin1 ());

          if (files.at (0).endsWith (".ch2"))
            {
              chrtr_handle = chrtr2_open_file (chrtr_name, &chrtr2_header, CHRTR2_READONLY);

              if (chrtr_handle < 0)
                {
                  QMessageBox::warning (this, tr ("Open CHRTR File"),
                                        tr ("The file ") + QDir::toNativeSeparators (chrtr_file_name) + 
                                        tr (" is not a CHRTR2 file or there was an error reading the file.") +
                                        tr ("  The error message returned was:\n\n") +
                                        QString (strerror (errno)));

                  return;
                }

              chrtr2_close_file (chrtr_handle);

              options->chrtr2 = NVTrue;
            }
          else
            {
              chrtr_handle = open_chrtr (chrtr_name, &chrtr_header);

              if (chrtr_handle < 0)
                {
                  QMessageBox::warning (this, tr ("Open CHRTR File"),
                                        tr ("The file ") + QDir::toNativeSeparators (chrtr_file_name) + 
                                        tr (" is not a CHRTR file or there was an error reading the file.") +
                                        tr ("  The error message returned was:\n\n") +
                                        QString (strerror (errno)));

                  return;
                }

              close_chrtr (chrtr_handle);

              options->chrtr2 = NVFalse;
            }
        }


      chrtr_file_edit->setText (chrtr_file_name);

      options->input_dir = fd->directory ().absolutePath ();


      //  If one hasn't been set, set the output TIFF filename.

      if (output_file_edit->text ().isEmpty ())
        { 
          QString output_file_name = chrtr_file_name + ".tif";
          output_file_edit->setText (output_file_name);
        }
    }
}



void startPage::slotOutputFileBrowse ()
{
  QFileDialog *fd = new QFileDialog (this, tr ("chrtrGeotiff Output File"));
  fd->setViewMode (QFileDialog::List);


  //  Always add the current working directory and the last used directory to the sidebar URLs in case we're running from the command line.
  //  This function is in the nvutility library.

  setSidebarUrls (fd, options->output_dir);


  QStringList filters;
  filters << tr ("Geotiff file (*.tif)");

  fd->setNameFilters (filters);
  fd->setFileMode (QFileDialog::AnyFile);
  fd->selectNameFilter (tr ("Geotiff file (*.tif)"));

  QStringList files;
  if (fd->exec () == QDialog::Accepted)
    {
      files = fd->selectedFiles ();

      QString output_file_name = files.at (0);

      options->output_dir = fd->directory ().absolutePath ();

      if (!output_file_name.isEmpty())
        {
          //  Add .tif to filename if not there.
            
          if (!output_file_name.endsWith (".tif")) output_file_name.append (".tif");

          output_file_edit->setText (output_file_name);
        }
    }
}


void startPage::slotAreaFileBrowse ()
{
  SHPHandle shpHandle;
  SHPObject *shape = NULL;
  int32_t type, numShapes;
  double minBounds[4], maxBounds[4];


  QFileDialog *fd = new QFileDialog (this, tr ("chrtrGeotiff Area File"));
  fd->setViewMode (QFileDialog::List);


  //  Always add the current working directory and the last used directory to the sidebar URLs in case we're running from the command line.
  //  This function is in the nvutility library.

  setSidebarUrls (fd, options->area_dir);


  QStringList filters;
  filters << tr ("Area file (*.ARE *.are *.afs *.shp *.SHP)");

  fd->setNameFilters (filters);
  fd->setFileMode (QFileDialog::ExistingFile);
  fd->selectNameFilter (tr ("Area file (*.ARE *.are *.afs *.shp *.SHP)"));


  QStringList files;
  if (fd->exec () == QDialog::Accepted)
    {
      files = fd->selectedFiles ();

      QString area_file_name = files.at (0);

      options->area_dir = fd->directory ().absolutePath ();

      if (!area_file_name.isEmpty())
        {
          if (area_file_name.endsWith (".shp", Qt::CaseInsensitive))
            {
              char shpname[1024];
              strcpy (shpname, area_file_name.toLatin1 ());


              //  Open shape file

              shpHandle = SHPOpen (shpname, "rb");

              if (shpHandle == NULL)
                {
                  QMessageBox::warning (this, tr ("chrtrGeotiff"), tr ("Cannot open shape file %1!").arg (area_file_name));
                  return;
                }
              else
                {
                  //  Get shape file header info

                  SHPGetInfo (shpHandle, &numShapes, &type, minBounds, maxBounds);


                  if (type != SHPT_POLYGON && type != SHPT_POLYGONZ && type != SHPT_POLYGONM &&
                      type != SHPT_ARC && type != SHPT_ARCZ && type != SHPT_ARCM)
                    {
                      QMessageBox::warning (this, tr ("chrtrGeotiff"), tr ("Shape file %1 is not a polygon or polyline file!").arg (area_file_name));
                      return;
                    }
                  else
                    {
                      //  Read only the first shape.

                      shape = SHPReadObject (shpHandle, 0);


                      //  Check the number of vertices.

                      if (shape->nVertices < 3)
                        {
                          SHPClose (shpHandle);
                          QMessageBox::warning (this, tr ("chrtrGeotiff"), tr ("Number of vertices (%1) of shape file %2 is too few for a polygon!").arg
                                                (shape->nVertices).arg (area_file_name));
                          return;
                        }


                      //  Read the vertices to take a shot at determining that this is a geographic polygon.

                      for (int32_t j = 0 ; j < shape->nVertices ; j++)
                        {
                          if (shape->padfX[j] < -360.0 || shape->padfX[j] > 360.0 || shape->padfY[j] < -90.0 || shape->padfY[j] > 90.0)
                            {
                              SHPDestroyObject (shape);
                              SHPClose (shpHandle);
                              QMessageBox::warning (this, tr ("chrtrGeotiff"), tr ("Shape file %1 does not appear to be geographic!").arg (area_file_name));
                              return;
                            }
                        }


                      SHPDestroyObject (shape);
                      SHPClose (shpHandle);
                    }
                }
            }

          area_file_edit->setText (area_file_name);
        }
    }
}
