#ifndef HELPER_H
#define HELPER_H

String cleanFilename(String filename){
  //turns all to lowercase and removes any non-ascii characters to prevent spiffs filename conflicts
  filename.toLowerCase();
  filename.replace(' ', '_');

  uint16_t featIndex = filename.indexOf("(feat.");
  if (featIndex != -1) {
    filename = filename.substring(0, featIndex);
    filename.trim();
  }
  
  for(int i=0; i<filename.length(); i++){
    if(!((96 < filename[i] && filename[i] < 123) || filename[i] == 95 || (47 < filename[i] && filename[i] < 58))){
      filename[i] = 'x';
    }
  }

  return filename;
}

#endif