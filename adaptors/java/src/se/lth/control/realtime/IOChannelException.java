/**
 * se.lth.control.realtime.IOChannelException.java
 *
 * Copyright (C) 2005-2019  Anders Blomdell <anders.blomdell@control.lth.se>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

package se.lth.control.realtime;

import java.io.IOException;

public class IOChannelException extends IOException {

  private static final long serialVersionUID = 1L;

  public IOChannelException(String kind, int index, String s) {
    super(kind + " channel #" + index + " " + s);
  }

}
