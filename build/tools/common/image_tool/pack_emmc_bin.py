#!/usr/bin/python3
# -*- coding: utf-8 -*-
import logging
import argparse
from os import path, stat
from array import array
from XmlParser import XmlParser

FORMAT = "%(levelname)s: %(message)s"
logging.basicConfig(level=logging.INFO, format=FORMAT)
LBA_SIZE = 512
MAX_WRITE_SIZE = 50 * 1024 * 1024


def parse_Args():
    parser = argparse.ArgumentParser(description="Create CVITEK device image")
    parser.add_argument("xml", help="path to partition xml")
    parser.add_argument("images_path", help="path to images")
    parser.add_argument(
        "output",
        metavar="output",
        type=str,
        help="the output folder for saving the data.bin and boot.bin",
    )
    parser.add_argument(
        "-m",
        "--max_write_size",
        help="max write buffer size when generating file. "
        "Increasing the size when speedup the procedue "
        "but it will use more system memory. "
        "Default is 50MB.",
        default=MAX_WRITE_SIZE,
    )
    parser.add_argument(
        "-v", "--verbose", help="increase output verbosity", action="store_true"
    )
    args = parser.parse_args()
    if args.verbose:
        logging.debug("Enable more verbose output")
        logging.getLogger().setLevel(level=logging.DEBUG)

    return args


def genDataBin(out_path, parts, images_path, max_write_size=50 * 1024 * 1024):
    gpt_path = path.join(images_path, "gpt.img")
    try:
        out = open(out_path, "wb")
    except Exception:
        logging.error("Create %s failed!", out_path)
        raise

    try:
        logging.debug("Packing %s", gpt_path)
        with open(gpt_path, "rb") as f:
            gpt = array("B")
            gpt_file_size = stat(gpt_path).st_size
            gpt.fromfile(f, gpt_file_size)
            # First partition will always at 8192 LBA which is defined by
            # tools/common/emmc_tool/mk_gpt.c
            for _ in range(8192 * LBA_SIZE - gpt_file_size):
                gpt.append(0xFF)
            gpt.tofile(out)
    except FileNotFoundError as e:
        logging.error("gpt.img is not exist!")
        raise e
    for i, p in enumerate(parts):
        file_array = array("B")
        file_path = path.join(images_path, p["file_name"])
        file_size = p["file_size"]
        logging.debug("Packing %s", p["label"])
        if file_size:
            with open(file_path, "rb") as f:
                file_array.fromfile(f, file_size)
        logging.info("Writing %s to file" % p["label"])
        file_array.tofile(out)
        # Only append 0xff when the partition is not the last partition.
        if i != len(parts) - 1:
            append_size = p["part_size"] - file_size
            # This part may seems stupid, but it works when image is too large
            # to keep content in memory.
            for j in range(0, append_size, max_write_size):
                append_byte = array(
                    "B", [0xFF for _ in range(min(max_write_size, append_size - j))]
                )
                append_byte.tofile(out)
    logging.info("generating Data.bin done!")


def genBootBin(out, images_path):
    file_path = path.join(images_path, "fip.bin")
    try:
        file_array = array("B")
        fip_size = stat(file_path).st_size
        with open(file_path, "rb") as f:
            file_array.fromfile(f, fip_size)
            for _ in range(0x800 * LBA_SIZE - stat(file_path).st_size):
                file_array.append(0xFF)
            f.seek(0)
            file_array.fromfile(f, fip_size)
        file_array.tofile(out)
        logging.info("generating Boot.bin done!")
    except FileNotFoundError as e:
        logging.error("fip.bin is not exist")
        raise e


def main():
    args = parse_Args()
    xmlParser = XmlParser(args.xml)
    parts = xmlParser.parse(args.images_path)
    out_path = path.join(args.output, "Data.bin")
    if xmlParser.getStorage() != "emmc":
        logging.error("Storage type %s is not supported" % xmlParser.getStorage())
        raise ValueError
    genDataBin(out_path, parts, args.images_path, args.max_write_size)
    out_path = path.join(args.output, "Boot.bin")
    try:
        fip_bin = open(out_path, "wb")
    except Exception:
        logging.error("Create %s failed!", out_path)
        raise
    fip_bin = open(out_path, "wb")
    genBootBin(fip_bin, args.images_path)


if __name__ == "__main__":
    main()
