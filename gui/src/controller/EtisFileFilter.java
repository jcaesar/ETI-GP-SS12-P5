
package controller;

import java.io.File;

/**
 *
 * @author Philip Becker-Ehmck
 */
public class EtisFileFilter extends javax.swing.filechooser.FileFilter {

    @Override
    public boolean accept(File file) {
        return file.isDirectory() || file.getAbsolutePath().endsWith(".etis");
    }

    @Override
    public String getDescription() {
        return "Text documents (*.etis)";
    }
}
